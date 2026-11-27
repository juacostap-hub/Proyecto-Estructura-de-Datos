// INTERFAZ GRÁFICA - Servidor local (el mismo de la fase 2, ahora conectado al juego)
// El programa en C++ abre un pequeño servidor en este mismo computador (http://localhost:8080) y el navegador
// muestra el juego (Pagina.h). Cada tecla o botón de la página llama a una acción de la clase Juego,
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

#include "Juego.h"            // Núcleo: la lógica del juego y sus estructuras
#include "Pagina.h"           // La página (HTML, CSS y JavaScript) guardada como texto
#include "Sprites.h"          // Las imágenes de Jonatan (caricatura sin fondo)
#include <iostream>           // Para escribir mensajes en la consola
#include <memory>             // std::unique_ptr
#include <cstdlib>            // std::system, para abrir el navegador
#include <cstring>            // Funciones de texto de C
#include <ctime>              // time(): semilla distinta en cada partida

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

static int leerLista(const string& texto, int lista[], int maximo) {   // "0,1,5,1,0" → números. Devuelve cuántos (maximo + 1 si sobran; -1 si hay algo raro)
    int n = 0;
    size_t inicio = 0;
    if (texto.empty()) return 0;
    while (inicio <= texto.size()) {
        size_t fin = texto.find(',', inicio);
        if (fin == string::npos) fin = texto.size();
        string parte = texto.substr(inicio, fin - inicio);
        char* resto = NULL;
        long v = strtol(parte.c_str(), &resto, 10);
        if (parte.empty() || *resto != '\0') return -1;          // No era un número
        if (n >= maximo) return maximo + 1;                      // Demasiados: el juego dirá que la ruta es muy larga
        lista[n++] = static_cast<int>(v);
        inicio = fin + 1;
    }
    return n;
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

static Respuesta json(const Resultado& r, Juego& j) {         // Respuesta JSON de cada acción: resultado + estado completo
    string s = string("{\"ok\":") + (r.ok ? "true" : "false") + ",\"mensaje\":" + comillas(r.mensaje);
    s += ",\"estado\":" + j.estadoJSON() + "}";
    Respuesta resp;
    resp.codigo = 200;
    resp.tipo = "application/json; charset=utf-8";
    resp.cuerpo = s;
    return resp;
}

static string reemplazar(string texto, const string& marca, const string& valor) {    // Cambia cada aparición de "marca" por "valor"
    size_t pos = 0;
    while ((pos = texto.find(marca, pos)) != string::npos) {
        texto.replace(pos, marca.size(), valor);
        pos += valor.size();
    }
    return texto;
}

static Respuesta atender(unique_ptr<Juego>& juego, const string& metodo, const string& ruta, const Parametros& p) {    // Decide qué hacer según la ruta
    Respuesta resp;
    resp.codigo = 200;
    resp.tipo = "text/plain; charset=utf-8";

    if (metodo == "GET" && ruta == "/") {                          // La página principal, con las imágenes de Jonatan pegadas
        static const string pagina = reemplazar(reemplazar(PAGINA_HTML, "{{JONATAN_CUERPO}}", SPRITE_JONATAN_CUERPO),
                                                "{{JONATAN_BUSTO}}", SPRITE_JONATAN_BUSTO);
        resp.tipo = "text/html; charset=utf-8";
        resp.cuerpo = pagina;
        return resp;
    }
    if (metodo == "GET" && ruta == "/api/estado") {                // El estado actual
        resp.tipo = "application/json; charset=utf-8";
        resp.cuerpo = juego->estadoJSON();
        return resp;
    }
    if (ruta == "/favicon.ico") {                                  // El navegador lo pide solo; no hay icono
        resp.codigo = 204;
        return resp;
    }
    if (metodo == "POST") {                                        // Acciones del jugador
        Juego& j = *juego;
        Resultado r;
        if (ruta == "/api/nueva") r = j.nuevaPartida(p.texto("nombre"), static_cast<unsigned>(time(NULL)));    // Semilla = la hora: cada partida es distinta
        else if (ruta == "/api/mover") r = j.mover(p.texto("dir").empty() ? ' ' : p.texto("dir")[0]);
        else if (ruta == "/api/usar") r = j.interactuar();
        else if (ruta == "/api/esperar") r = j.esperar();
        else if (ruta == "/api/sentar") r = j.sentar(p.entero("mesa", -1));
        else if (ruta == "/api/cerrar") r = j.cerrarTemprano();
        else if (ruta == "/api/ruta") r = j.verRuta(p.entero("a", -1), p.entero("b", -1));             // GPS
        else if (ruta == "/api/salir" || ruta == "/api/desviar") {                                      // Ruta trazada por el jugador
            int nodos[MAX_RUTA];
            int largo = leerLista(p.texto("ruta"), nodos, MAX_RUTA);
            if (largo < 0) r = Resultado(false, "La ruta no se entendió.");
            else if (ruta == "/api/salir") r = j.salir(p.entero("a", -1), p.entero("b", -1), nodos, largo);
            else r = j.desviar(nodos, largo);
        }
        else if (ruta == "/api/urgente") r = j.responderUrgente(p.entero("acepta", 0) == 1);
        else if (ruta == "/api/terminarNoche") r = j.terminarNoche();
        else if (ruta == "/api/mejora") r = j.comprarMejora(p.entero("tipo", -1));
        else if (ruta == "/api/siguiente") r = j.siguienteJornada();
        else if (ruta == "/api/inicio") {                          // Volver a la pantalla de inicio (juego nuevo, mismo ranking)
            juego.reset(new Juego());
            return json(Resultado(true, ""), *juego);
        }
        else {
            resp.codigo = 404;
            resp.cuerpo = "No existe";
            return resp;
        }
        return json(r, j);
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

static void responder(Socket s, const string& datos, size_t finEncabezado, size_t largoCuerpo, unique_ptr<Juego>& juego) {    // Atiende una petición ya completa
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
        resp = atender(juego, metodo, ruta, p);
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
    unique_ptr<Juego> juego(new Juego());                          // Todos los datos del juego

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
    cout << "\n  ORDERSTACK: RESTAURANT RUSH - Juego listo en " << url << "\n";
    cout << "  Se abre sola en el navegador. Para terminar el programa presione Ctrl + C o cierre esta ventana.\n\n" << flush;

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
                responder(con[i].s, con[i].datos, finEnc, largo, juego);
                CERRAR_SOCKET(con[i].s);                           // Una petición por conexión (Connection: close)
                con[i].activa = false;
            }
        }
    }
    return 0;
}
