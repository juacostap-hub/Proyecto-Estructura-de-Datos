// INTERFAZ GRÁFICA - Servidor local
// El programa en C++ abre un pequeño servidor en este mismo computador (http://localhost:8080) y el navegador
// muestra la interfaz (Pagina.h). Cada botón de la página llama a una operación de la clase Cafeteria,
// así que TODA la lógica y TODAS las estructuras de datos siguen siendo las de C++.
// Este servidor está hecho con los "sockets" del sistema (los mismos de Windows, Linux y Mac), sin librerías externas
// y sin hilos: atiende una conexión a la vez usando select(), que sirve de sobra para un solo usuario.

#ifdef _WIN32
#include <winsock2.h>                 // Sockets de Windows (se enlaza con -lws2_32)
typedef SOCKET Socket;                // Tipo de un socket
#define CERRAR_SOCKET closesocket     // Cerrar un socket
#define SOCKET_INVALIDO INVALID_SOCKET
#else
#include <sys/types.h>                // Sockets de Linux, Mac y Colab
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int Socket;
#define CERRAR_SOCKET close
#define SOCKET_INVALIDO (-1)
#endif

#include "Cafeteria.h"        // Núcleo: estructuras y operaciones
#include "Pagina.h"           // La página (HTML, CSS y JavaScript) guardada como texto
#include <iostream>           // Para escribir mensajes en la consola
#include <memory>             // std::unique_ptr
#include <cstdlib>            // std::system, para abrir el navegador
#include <cstring>            // Funciones de texto de C

using namespace std;

// ---------------------------------------------------------------
// Datos que llegan en cada petición (formulario)
// ---------------------------------------------------------------

struct Parametros {                                // Pares clave=valor de una petición (máximo 16)
    static const int MAX = 16;
    string clave[MAX];
    string valor[MAX];
    int n;
    Parametros() : n(0) {}

    void agregar(const string& c, const string& v) {         // Guarda un par clave=valor
        if (n < MAX) { clave[n] = c; valor[n] = v; n++; }
    }
    string texto(const string& c) const {                    // Devuelve el valor de una clave sin espacios sobrantes ("" si no existe)
        for (int i = 0; i < n; i++) {
            if (clave[i] == c) {
                const string& s = valor[i];
                size_t a = s.find_first_not_of(" \t\r\n");
                if (a == string::npos) return "";
                size_t b = s.find_last_not_of(" \t\r\n");
                return s.substr(a, b - a + 1);
            }
        }
        return "";
    }
    int entero(const string& c, int porDefecto) const {      // Devuelve un número entero (o porDefecto si no hay o no es número)
        string t = texto(c);
        if (t.empty()) return porDefecto;
        char* fin = NULL;
        long v = strtol(t.c_str(), &fin, 10);                // Convierte el texto a número
        if (*fin != '\0') return porDefecto;                 // Si sobraron letras, no era un número
        return static_cast<int>(v);
    }
    double decimal(const string& c) const {                  // Devuelve un número con decimales (0 si no es número)
        string t = texto(c);
        if (t.empty()) return 0;
        char* fin = NULL;
        double v = strtod(t.c_str(), &fin);                  // Convierte el texto a número con decimales
        if (*fin != '\0') return 0;                          // Si sobraron letras, no era un número
        return v;
    }
};

static int valorHex(char c) {                      // Convierte un dígito hexadecimal a número (o -1)
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static string decodificar(const string& s) {       // Deshace la codificación de formularios: %C3%A1 → á, + → espacio
    string r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') r += ' ';
        else if (s[i] == '%' && i + 2 < s.size() && valorHex(s[i + 1]) >= 0 && valorHex(s[i + 2]) >= 0) {
            r += static_cast<char>(valorHex(s[i + 1]) * 16 + valorHex(s[i + 2]));
            i += 2;
        } else r += s[i];
    }
    return r;
}

static void leerParametros(const string& datos, Parametros& p) {    // Separa "a=1&b=2" en pares
    size_t inicio = 0;
    while (inicio <= datos.size()) {
        size_t fin = datos.find('&', inicio);
        if (fin == string::npos) fin = datos.size();
        string par = datos.substr(inicio, fin - inicio);
        if (!par.empty()) {
            size_t igual = par.find('=');
            if (igual == string::npos) p.agregar(decodificar(par), "");
            else p.agregar(decodificar(par.substr(0, igual)), decodificar(par.substr(igual + 1)));
        }
        inicio = fin + 1;
    }
}

// ---------------------------------------------------------------
// Respuestas
// ---------------------------------------------------------------

struct Respuesta {                                 // Lo que se le devuelve al navegador
    int codigo;                                    // 200 = bien, 204 = sin contenido, 404 = no existe
    string tipo;                                   // Tipo de contenido
    string cuerpo;                                 // El contenido
};

static string comillas(const string& s) {          // Texto entre comillas para JSON (solo para mensajes)
    string r = "\"";
    for (unsigned char c : s) {
        if (c == '"') r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c < 0x20) r += ' ';
        else r += static_cast<char>(c);
    }
    return r + "\"";
}

static Respuesta json(const Resultado& r, Cafeteria& c) {    // Respuesta JSON de cada acción: resultado + estado completo
    string s = string("{\"ok\":") + (r.ok ? "true" : "false") + ",\"mensaje\":" + comillas(r.mensaje);
    if (!r.extra.empty()) s += "," + r.extra;
    s += ",\"estado\":" + c.estadoJSON() + "}";
    Respuesta resp;
    resp.codigo = 200;
    resp.tipo = "application/json; charset=utf-8";
    resp.cuerpo = s;
    return resp;
}

static Respuesta atender(unique_ptr<Cafeteria>& cafe, const string& metodo, const string& ruta, const Parametros& p) {    // Decide qué hacer según la ruta
    Respuesta resp;
    resp.codigo = 200;
    resp.tipo = "text/plain; charset=utf-8";

    if (metodo == "GET" && ruta == "/") {                          // La página principal
        resp.tipo = "text/html; charset=utf-8";
        resp.cuerpo = PAGINA_HTML;
        return resp;
    }
    if (metodo == "GET" && ruta == "/api/estado") {                // El estado actual
        resp.tipo = "application/json; charset=utf-8";
        resp.cuerpo = cafe->estadoJSON();
        return resp;
    }
    if (ruta == "/favicon.ico") {                                  // El navegador lo pide solo; no hay icono
        resp.codigo = 204;
        return resp;
    }
    if (metodo == "POST") {                                        // Acciones de los botones
        Cafeteria& c = *cafe;
        Resultado r;
        r.ok = false;
        if (ruta == "/api/cliente") r = c.registrarCliente(p.texto("nombre"), p.entero("personas", 0), p.entero("tarjeta", 0));
        else if (ruta == "/api/llamar") r = c.llamarSiguiente();
        else if (ruta == "/api/comanda") r = c.tomarComanda(p.entero("mesa", -1), p.entero("plato", -1));
        else if (ruta == "/api/deshacer") r = c.deshacerProducto(p.entero("mesa", -1));
        else if (ruta == "/api/pagar") r = c.pagarCuenta(p.entero("mesa", -1));
        else if (ruta == "/api/domicilio/agregar") r = c.agregarProductoDomicilio(p.entero("plato", -1));
        else if (ruta == "/api/domicilio/deshacer") r = c.deshacerDomicilio();
        else if (ruta == "/api/domicilio/confirmar") r = c.confirmarDomicilio(p.texto("cliente"), p.texto("direccion"));
        else if (ruta == "/api/personal/agregar") r = c.agregarPersonal(p.texto("tipo") == "mesero", p.texto("nombre"));
        else if (ruta == "/api/personal/liberar") r = c.liberarPersonal(p.texto("tipo") == "mesero", p.entero("id", -1));
        else if (ruta == "/api/menu/agregar") r = c.agregarPlato(p.texto("nombre"), p.decimal("precio"));
        else if (ruta == "/api/menu/quitar") r = c.quitarUltimoPlato();
        else if (ruta == "/api/menu/buscar") {                     // Usa buscar() del arreglo dinámico
            int pos = c.buscarPlato(p.texto("nombre"));
            r.ok = pos >= 0;
            r.mensaje = pos >= 0 ? "Encontrado en la posición [" + to_string(pos) + "] del arreglo." : "No existe ese plato en el menú.";
            r.extra = "\"posicion\":" + to_string(pos);
        }
        else if (ruta == "/api/mesas/agregar") r = c.agregarMesa(p.entero("capacidad", 0));
        else if (ruta == "/api/reiniciar") {                       // Vuelve a los datos iniciales
            cafe.reset(new Cafeteria());
            r.ok = true;
            r.mensaje = "Se reiniciaron los datos de la cafetería.";
            return json(r, *cafe);
        }
        else {
            resp.codigo = 404;
            resp.cuerpo = "No existe";
            return resp;
        }
        return json(r, c);
    }
    resp.codigo = 404;
    resp.cuerpo = "No existe";
    return resp;
}

// ---------------------------------------------------------------
// Conexiones
// ---------------------------------------------------------------

struct Conexion {                                  // Una conexión abierta con el navegador
    Socket s;
    bool activa;
    string datos;                                  // Lo que ha llegado hasta ahora
    Conexion() : s(SOCKET_INVALIDO), activa(false) {}
};

const int MAX_CONEXIONES = 30;                     // El navegador abre unas pocas conexiones a la vez

static string enMinusculas(string s) {
    for (size_t i = 0; i < s.size(); i++) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = static_cast<char>(s[i] - 'A' + 'a');
    return s;
}

static bool peticionCompleta(const string& d, size_t& finEncabezado, size_t& largoCuerpo) {    // ¿Ya llegó la petición completa?
    size_t fin = d.find("\r\n\r\n");
    if (fin == string::npos) return false;                         // Todavía faltan los encabezados
    finEncabezado = fin + 4;
    string cabeza = enMinusculas(d.substr(0, fin));
    largoCuerpo = 0;
    size_t cl = cabeza.find("content-length:");
    if (cl != string::npos) largoCuerpo = static_cast<size_t>(atoi(cabeza.c_str() + cl + 15));
    return d.size() >= finEncabezado + largoCuerpo;                // ¿Llegó también el cuerpo?
}

static void enviarTodo(Socket s, const string& datos) {            // Envía todo el texto (puede requerir varios envíos)
    size_t enviado = 0;
    while (enviado < datos.size()) {
        int n = ::send(s, datos.c_str() + enviado, static_cast<int>(datos.size() - enviado), 0);
        if (n <= 0) return;
        enviado += static_cast<size_t>(n);
    }
}

static void responder(Socket s, const string& datos, size_t finEncabezado, size_t largoCuerpo, unique_ptr<Cafeteria>& cafe) {    // Atiende una petición ya completa
    size_t finLinea = datos.find("\r\n");
    string linea = datos.substr(0, finLinea);                      // Ejemplo: "POST /api/llamar HTTP/1.1"
    size_t e1 = linea.find(' ');
    size_t e2 = (e1 == string::npos) ? string::npos : linea.find(' ', e1 + 1);
    Respuesta resp;
    if (e1 == string::npos || e2 == string::npos) {
        resp.codigo = 400; resp.tipo = "text/plain"; resp.cuerpo = "Peticion no valida";
    } else {
        string metodo = linea.substr(0, e1);
        string destino = linea.substr(e1 + 1, e2 - e1 - 1);
        string ruta = destino, consulta;
        size_t q = destino.find('?');
        if (q != string::npos) { ruta = destino.substr(0, q); consulta = destino.substr(q + 1); }
        Parametros p;
        leerParametros(consulta, p);                               // Datos en la dirección
        leerParametros(datos.substr(finEncabezado, largoCuerpo), p);    // Datos en el cuerpo (formulario)
        resp = atender(cafe, metodo, ruta, p);
    }
    string estado = resp.codigo == 200 ? "OK" : resp.codigo == 204 ? "No Content" : resp.codigo == 404 ? "Not Found" : "Bad Request";
    string cabecera = "HTTP/1.1 " + to_string(resp.codigo) + " " + estado + "\r\nContent-Type: " + resp.tipo +
                      "\r\nContent-Length: " + to_string(resp.cuerpo.size()) + "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
    enviarTodo(s, cabecera + resp.cuerpo);
}

int ejecutarInterfazGrafica(bool abrirNavegador) {
#ifdef _WIN32
    WSADATA wsa;                                                   // Prepara los sockets en Windows
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "No se pudo iniciar la red de Windows (Winsock).\n";
        return 1;
    }
#endif
    unique_ptr<Cafeteria> cafe(new Cafeteria());                   // Todos los datos del programa

    Socket escucha = ::socket(AF_INET, SOCK_STREAM, 0);            // Socket que espera al navegador
    if (escucha == SOCKET_INVALIDO) {
        cout << "No se pudo crear el socket.\n";
        return 1;
    }
#ifndef _WIN32
    int uno = 1;
    setsockopt(escucha, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&uno), sizeof(uno));    // Permite reiniciar el programa enseguida
#endif
    int puerto = 0;                                                // Busca un puerto libre entre 8080 y 8090
    for (int p = 8080; p <= 8090; p++) {
        sockaddr_in dir;
        memset(&dir, 0, sizeof(dir));
        dir.sin_family = AF_INET;
        dir.sin_addr.s_addr = inet_addr("127.0.0.1");              // Solo este computador
        dir.sin_port = htons(static_cast<unsigned short>(p));
        if (::bind(escucha, reinterpret_cast<sockaddr*>(&dir), sizeof(dir)) == 0) { puerto = p; break; }
    }
    if (puerto == 0 || ::listen(escucha, 16) != 0) {
        cout << "No se pudo abrir ningun puerto entre 8080 y 8090. Cierre otros programas e intente de nuevo.\n";
        return 1;
    }
    string url = "http://localhost:" + to_string(puerto);
    cout << "\n  CAFETERIA - Interfaz grafica lista en " << url << "\n";
    cout << "  Se abre sola en el navegador. Para terminar el programa presione Ctrl + C o cierre esta ventana.\n\n";

    if (abrirNavegador) {                                          // Abre el navegador (las conexiones esperan en fila hasta que el servidor las atienda)
#ifdef _WIN32
        string cmd = "start \"\" " + url;
#elif defined(__APPLE__)
        string cmd = "open " + url;
#else
        string cmd = "xdg-open " + url + " >/dev/null 2>&1 &";
#endif
        std::system(cmd.c_str());
    }

    Conexion con[MAX_CONEXIONES];                                  // Conexiones abiertas con el navegador
    int siguienteReemplazo = 0;                                    // Si se llenan, se cierra una vieja para dar lugar
    while (true) {                                                 // Atiende conexiones hasta que se cierre el programa
        fd_set lectura;
        FD_ZERO(&lectura);
        FD_SET(escucha, &lectura);
        int maximo = static_cast<int>(escucha);
        for (int i = 0; i < MAX_CONEXIONES; i++) {
            if (con[i].activa) {
                FD_SET(con[i].s, &lectura);
                if (static_cast<int>(con[i].s) > maximo) maximo = static_cast<int>(con[i].s);
            }
        }
        int listos = ::select(maximo + 1, &lectura, NULL, NULL, NULL);    // Espera hasta que algo llegue (no se queda pegado en una conexión sin datos)
        if (listos <= 0) continue;

        if (FD_ISSET(escucha, &lectura)) {                         // Nueva conexión del navegador
            Socket nuevo = ::accept(escucha, NULL, NULL);
            if (nuevo != SOCKET_INVALIDO) {
                int libre = -1;
                for (int i = 0; i < MAX_CONEXIONES; i++) if (!con[i].activa) { libre = i; break; }
                if (libre == -1) {                                 // No hay lugar: se cierra una conexión vieja
                    libre = siguienteReemplazo;
                    siguienteReemplazo = (siguienteReemplazo + 1) % MAX_CONEXIONES;
                    CERRAR_SOCKET(con[libre].s);
                }
                con[libre].s = nuevo;
                con[libre].activa = true;
                con[libre].datos.clear();
            }
        }
        for (int i = 0; i < MAX_CONEXIONES; i++) {                 // Conexiones que mandaron datos
            if (!con[i].activa || !FD_ISSET(con[i].s, &lectura)) continue;
            char buf[8192];
            int n = ::recv(con[i].s, buf, sizeof(buf), 0);
            if (n <= 0) {                                          // El navegador cerró la conexión
                CERRAR_SOCKET(con[i].s);
                con[i].activa = false;
                continue;
            }
            con[i].datos.append(buf, static_cast<size_t>(n));
            size_t finEnc = 0, largo = 0;
            if (con[i].datos.size() > 200000) {                    // Petición demasiado grande: se descarta
                CERRAR_SOCKET(con[i].s);
                con[i].activa = false;
            } else if (peticionCompleta(con[i].datos, finEnc, largo)) {
                responder(con[i].s, con[i].datos, finEnc, largo, cafe);
                CERRAR_SOCKET(con[i].s);                           // Una petición por conexión (Connection: close)
                con[i].activa = false;
            }
        }
    }
    return 0;
}
