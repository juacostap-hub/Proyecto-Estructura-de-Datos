#include "Cafeteria.h"
#include <sstream>        // Para armar el texto JSON (std::ostringstream)
#include <iomanip>        // Para dar formato a los precios con decimales
#include <cmath>          // Para floor()

using namespace std;

// ---------------------------------------------------------------
// Funciones de apoyo para armar el JSON
// ---------------------------------------------------------------

static string esc(const string& s) {               // Prepara un texto para ponerlo entre comillas en JSON
    string r;
    for (unsigned char c : s) {                    // Recorre cada carácter
        if (c == '"') r += "\\\"";                 // Comilla doble → \"
        else if (c == '\\') r += "\\\\";           // Barra invertida (se duplica)
        else if (c == '\n') r += "\\n";            // Salto de línea → \n
        else if (c == '\r') r += "\\r";
        else if (c == '\t') r += "\\t";
        else if (c < 0x20) r += ' ';               // Otros caracteres de control se cambian por espacio
        else r += static_cast<char>(c);            // Los demás (incluidas tildes en UTF-8) pasan igual
    }
    return r;
}

static string num(double v) {                      // Número sin decimales si es entero (25000), con 2 decimales si no (12.50)
    if (v == floor(v)) return to_string(static_cast<long long>(v));
    ostringstream o;
    o << fixed << setprecision(2) << v;
    return o.str();
}

static string pesos(double v) {                    // Precio para mensajes: 30000 → 30.000
    string n = num(v), r;
    size_t punto = n.find('.');
    string entera = (punto == string::npos) ? n : n.substr(0, punto);
    string resto = (punto == string::npos) ? "" : "," + n.substr(punto + 1);
    int c = 0;
    for (int i = static_cast<int>(entera.size()) - 1; i >= 0; i--) {     // De derecha a izquierda, un punto cada 3 cifras
        r = entera[i] + r;
        if (++c % 3 == 0 && i > 0) r = "." + r;
    }
    return "$" + r + resto;
}

static string jsonCliente(Cliente& c) {            // Un cliente de las filas
    ostringstream o;
    o << "{\"turno\":" << c.turno << ",\"nombre\":\"" << esc(c.nombre) << "\",\"personas\":" << c.personas
      << ",\"tarjeta\":" << c.tarjeta << ",\"tarjetaNombre\":\"" << esc(c.nombre_tarjeta()) << "\"}";
    return o.str();
}

static string nombreEmpleado(const GrupoPersonal& g, int id) {    // Busca el nombre de un empleado por su ID
    for (int i = 0; i < g.getCola().tamano(); i++) {
        const Empleado* e = g.getCola().obtener(i);
        if (e->id == id) return e->nombre;
    }
    return "";
}

static string jsonGrupo(const GrupoPersonal& g) {  // Una cola circular completa: posiciones del arreglo, frente, final y orden de turnos
    const ColaCircular& c = g.getCola();
    int cap = c.getCapacidad(), n = c.tamano(), fr = c.getFrente(), fi = c.getFinal();
    ostringstream o;
    o << "{\"capacidad\":" << cap << ",\"cantidad\":" << n << ",\"frente\":" << fr << ",\"final\":" << fi
      << ",\"libres\":" << g.cantidadLibres() << ",\"ranuras\":[";
    for (int k = 0; k < cap; k++) {                // Cada posición del arreglo (esté usada o no)
        bool usada = ((k - fr + cap) % cap) < n;   // Una posición está en uso si está entre "frente" y "final"
        if (k > 0) o << ",";
        o << "{\"usada\":" << (usada ? "true" : "false");
        if (usada) {
            const Empleado& e = c.ranura(k);
            o << ",\"id\":" << e.id << ",\"nombre\":\"" << esc(e.nombre) << "\",\"ocupado\":" << (e.ocupado ? "true" : "false")
              << ",\"servicios\":" << e.servicios;
        }
        o << "}";
    }
    o << "],\"orden\":[";                          // Los mismos empleados, pero en orden de turno (el primero es el siguiente)
    for (int i = 0; i < n; i++) {
        const Empleado* e = c.obtener(i);
        if (i > 0) o << ",";
        o << "{\"id\":" << e->id << ",\"nombre\":\"" << esc(e->nombre) << "\",\"ocupado\":" << (e->ocupado ? "true" : "false")
          << ",\"servicios\":" << e->servicios << "}";
    }
    o << "]}";
    return o.str();
}

static string jsonPila(PilaPedidos& p, double& total) {    // Una pila de pedidos, del fondo al tope
    ostringstream o;
    total = 0;
    o << "[";
    for (int i = 0; i < p.cantidad(); i++) {
        Pedidos pe = p.obtener(i);
        if (i > 0) o << ",";
        o << "{\"nombre\":\"" << esc(pe.platoPedido.nombre) << "\",\"precio\":" << num(pe.platoPedido.precio) << "}";
        total += pe.platoPedido.precio;
    }
    o << "]";
    return o.str();
}

static string jsonFila(Cola& f) {                  // La fila común, del primero al último
    ostringstream o;
    o << "[";
    for (int i = 0; i < f.longitud(); i++) {
        Cliente c;
        f.obtenerEn(i, c);
        if (i > 0) o << ",";
        o << jsonCliente(c);
    }
    o << "]";
    return o.str();
}

static string jsonFila(ColaPrioridad& f) {         // La fila VIP, del primero al último
    ostringstream o;
    o << "[";
    for (int i = 0; i < f.longitud(); i++) {
        Cliente c;
        f.obtenerEn(i, c);
        if (i > 0) o << ",";
        o << jsonCliente(c);
    }
    o << "]";
    return o.str();
}

static Resultado fallo(const string& m) { return Resultado{false, m, ""}; }       // Resultado de error
static Resultado exito(const string& m, const string& extra = "") { return Resultado{true, m, extra}; }   // Resultado correcto

// ---------------------------------------------------------------
// Operaciones
// ---------------------------------------------------------------

Cafeteria::Cafeteria() : meseros("Mesero", "Meseros", 6), domiciliarios("Domiciliario", "Domiciliarios", 4) {
    cargarDatosIniciales(meseros, domiciliarios, rest);       // Mismos datos de ejemplo que la consola
}

Resultado Cafeteria::registrarCliente(const string& nombre, int personas, int tarjeta) {
    if (nombre.empty()) return fallo("Escriba el nombre del cliente.");
    if (personas <= 0) return fallo("La cantidad de personas debe ser mayor a 0.");
    if (personas > capacidadMaxima(rest))                       // Debe existir alguna mesa donde quepan
        return fallo("Ninguna mesa tiene capacidad para " + to_string(personas) + " personas (máximo: " + to_string(capacidadMaxima(rest)) + ").");
    if (tarjeta < SIN_TARJETA || tarjeta > PRESTIGIO) return fallo("Tarjeta no válida.");
    Cliente c(nombre, personas, tarjeta, filas.turno);          // Crea el cliente
    filas.turno++;                                              // El siguiente tendrá el turno siguiente
    if (c.esvip()) {                                            // Con tarjeta: fila VIP (cola con prioridad)
        filas.vip.encolar(c);
        return exito(nombre + " entra a la fila VIP (" + c.nombre_tarjeta() + ").");
    }
    filas.comun.encolar(c);                                     // Sin tarjeta: fila común (cola normal)
    return exito(nombre + " entra a la fila común.");
}

Resultado Cafeteria::llamarSiguiente() {
    Cliente c;
    bool deVip = false;
    if (!siguienteCliente(filas, c, deVip)) return fallo("No hay clientes esperando.");
    if (meseros.cantidadLibres() == 0) return fallo("No hay meseros libres. Libere uno para poder llamar al siguiente cliente.");
    int pos = buscarMesaLibre(rest, c.personas);                // Mesa libre más pequeña donde quepa el grupo
    if (pos == -1) return fallo("No hay una mesa libre para " + to_string(c.personas) + " persona(s). El cliente sigue esperando.");
    Cliente salida;
    if (deVip) { filas.vip.desencolar(salida); filas.vipSeguidos++; }        // Sale de su fila y se actualiza la cuenta de VIP seguidos
    else { filas.comun.desencolar(salida); filas.vipSeguidos = 0; }
    Empleado mesero;
    meseros.asignarSiguienteLibre(mesero);                      // Cola circular: siguiente mesero libre
    rest.mesas.cambiarEstado(pos, true);                        // La mesa pasa a ocupada
    rest.meseroDeMesa[pos] = mesero.id;                         // Se anota qué mesero la atiende
    return exito(salida.nombre + " pasa a la mesa " + to_string(rest.mesas.obtener(pos).id) + " con " + mesero.nombre + ".");
}

Resultado Cafeteria::tomarComanda(int idMesa, int indicePlato) {
    int pos = rest.mesas.buscar(idMesa);
    if (pos == -1) return fallo("No existe esa mesa.");
    if (!rest.mesas.obtener(pos).ocupada) return fallo("La mesa " + to_string(idMesa) + " no está ocupada.");
    if (indicePlato < 0 || indicePlato >= rest.menu.longitud()) return fallo("Plato no válido.");
    Plato p = rest.menu.obtener(indicePlato);
    rest.comandas[pos].agregar(Pedidos(p, idMesa));             // APILA el pedido en la comanda de la mesa
    return exito("Agregado a la mesa " + to_string(idMesa) + ": " + p.nombre + ".");
}

Resultado Cafeteria::deshacerProducto(int idMesa) {
    int pos = rest.mesas.buscar(idMesa);
    if (pos == -1) return fallo("No existe esa mesa.");
    if (rest.comandas[pos].estaVacia()) return fallo("La mesa " + to_string(idMesa) + " no tiene productos en la comanda.");
    string nombre = rest.comandas[pos].top().platoPedido.nombre;    // Mira el tope de la pila
    rest.comandas[pos].borrar();                                // DESAPILA: sale el último que entró
    return exito("Se deshizo: " + nombre + ".");
}

Resultado Cafeteria::pagarCuenta(int idMesa) {
    int pos = rest.mesas.buscar(idMesa);
    if (pos == -1) return fallo("No existe esa mesa.");
    if (!rest.mesas.obtener(pos).ocupada) return fallo("La mesa " + to_string(idMesa) + " no está ocupada.");
    double total = 0;
    ostringstream lineas;
    bool primera = true;
    while (!rest.comandas[pos].estaVacia()) {                   // Se desapila toda la comanda: sale del último producto al primero
        Plato p = rest.comandas[pos].top().platoPedido;
        if (!primera) lineas << ",";
        primera = false;
        lineas << "{\"nombre\":\"" << esc(p.nombre) << "\",\"precio\":" << num(p.precio) << "}";
        total += p.precio;
        rest.comandas[pos].borrar();
    }
    string mesero = nombreEmpleado(meseros, rest.meseroDeMesa[pos]);
    rest.ventasTotal += total;                                  // Se suma a las ventas del día
    rest.cuentasSalon++;
    if (rest.meseroDeMesa[pos] != 0) {                          // El mesero queda libre
        meseros.liberar(rest.meseroDeMesa[pos]);
        rest.meseroDeMesa[pos] = 0;
    }
    rest.mesas.cambiarEstado(pos, false);                       // La mesa queda libre
    string extra = "\"cuenta\":{\"titulo\":\"Mesa " + to_string(idMesa) + "\",\"mesero\":\"" + esc(mesero) + "\",\"lineas\":[" + lineas.str() + "],\"total\":" + num(total) + "}";
    return exito("Cuenta de la mesa " + to_string(idMesa) + " pagada: " + pesos(total) + ". Mesa y mesero libres.", extra);
}

Resultado Cafeteria::agregarProductoDomicilio(int indicePlato) {
    if (indicePlato < 0 || indicePlato >= rest.menu.longitud()) return fallo("Plato no válido.");
    Plato p = rest.menu.obtener(indicePlato);
    rest.comandaDomicilio.agregar(Pedidos(p, 0));               // Se apila (mesa 0 = no es de ninguna mesa)
    return exito("Agregado al domicilio: " + p.nombre + ".");
}

Resultado Cafeteria::deshacerDomicilio() {
    if (rest.comandaDomicilio.estaVacia()) return fallo("El pedido a domicilio no tiene productos.");
    string nombre = rest.comandaDomicilio.top().platoPedido.nombre;
    rest.comandaDomicilio.borrar();
    return exito("Se deshizo: " + nombre + ".");
}

Resultado Cafeteria::confirmarDomicilio(const string& cliente, const string& direccion) {
    if (rest.comandaDomicilio.estaVacia()) return fallo("El pedido a domicilio no tiene productos.");
    if (cliente.empty() || direccion.empty()) return fallo("Escriba el nombre y la dirección del cliente.");
    if (domiciliarios.cantidadLibres() == 0) return fallo("No hay domiciliarios libres. El pedido sigue abierto: libere uno e intente de nuevo.");
    double total = 0;
    ostringstream lineas;
    bool primera = true;
    while (!rest.comandaDomicilio.estaVacia()) {                // Se desapila todo el pedido
        Plato p = rest.comandaDomicilio.top().platoPedido;
        if (!primera) lineas << ",";
        primera = false;
        lineas << "{\"nombre\":\"" << esc(p.nombre) << "\",\"precio\":" << num(p.precio) << "}";
        total += p.precio;
        rest.comandaDomicilio.borrar();
    }
    rest.ventasTotal += total;
    rest.pedidosDomicilio++;
    Empleado d;
    domiciliarios.asignarSiguienteLibre(d);                     // Cola circular: siguiente domiciliario libre
    string extra = "\"cuenta\":{\"titulo\":\"Domicilio\",\"mesero\":\"" + esc(d.nombre) + "\",\"cliente\":\"" + esc(cliente) + "\",\"direccion\":\"" + esc(direccion) + "\",\"lineas\":[" + lineas.str() + "],\"total\":" + num(total) + "}";
    return exito("Domicilio para " + cliente + " confirmado (" + pesos(total) + "). Lo lleva " + d.nombre + ".", extra);
}

Resultado Cafeteria::agregarPersonal(bool esMesero, const string& nombre) {
    GrupoPersonal& g = esMesero ? meseros : domiciliarios;
    if (nombre.empty()) return fallo("Escriba el nombre.");
    if (!g.agregar(nombre)) return fallo("No se puede agregar: el grupo está lleno (máximo " + to_string(g.getCola().getCapacidad()) + ").");
    return exito(g.getTipo() + " agregado: " + nombre + ".");
}

Resultado Cafeteria::liberarPersonal(bool esMesero, int id) {
    GrupoPersonal& g = esMesero ? meseros : domiciliarios;
    if (!g.liberar(id)) return fallo("No existe un empleado con ese ID.");
    return exito("Quedó libre: " + nombreEmpleado(g, id) + ".");
}

Resultado Cafeteria::agregarPlato(const string& nombre, double precio) {
    if (nombre.empty()) return fallo("Escriba el nombre del plato.");
    if (precio <= 0) return fallo("El precio debe ser mayor a 0.");
    rest.menu.agregar(Plato(rest.siguienteIdPlato, nombre, precio));    // Arreglo dinámico: crece si hace falta
    rest.siguienteIdPlato++;
    return exito("Plato agregado al menú: " + nombre + ".");
}

Resultado Cafeteria::quitarUltimoPlato() {
    if (rest.menu.longitud() == 0) return fallo("El menú está vacío.");
    Plato ultimo = rest.menu.obtener(rest.menu.longitud() - 1);
    rest.menu.borrar();
    return exito("Se quitó del menú: " + ultimo.nombre + ".");
}

int Cafeteria::buscarPlato(const string& nombre) {
    return rest.menu.buscar(nombre);
}

Resultado Cafeteria::agregarMesa(int capacidad) {
    if (rest.mesas.longitud() >= MAX_MESAS) return fallo("Se alcanzó el máximo de " + to_string(MAX_MESAS) + " mesas.");
    if (capacidad <= 0) return fallo("La capacidad debe ser mayor a 0.");
    rest.mesas.agregar(Mesa(rest.siguienteIdMesa, capacidad));
    int id = rest.siguienteIdMesa;
    rest.siguienteIdMesa++;
    return exito("Mesa " + to_string(id) + " agregada (" + to_string(capacidad) + " personas).");
}

// ---------------------------------------------------------------
// Estado completo en JSON
// ---------------------------------------------------------------

string Cafeteria::estadoJSON() {
    ostringstream o;
    o << "{\"menu\":[";
    for (int i = 0; i < rest.menu.longitud(); i++) {            // Platos del menú (arreglo dinámico)
        Plato p = rest.menu.obtener(i);
        if (i > 0) o << ",";
        o << "{\"i\":" << i << ",\"id\":" << p.id << ",\"nombre\":\"" << esc(p.nombre) << "\",\"precio\":" << num(p.precio) << "}";
    }
    o << "],\"menuTamano\":" << rest.menu.capacidadActual() << ",\"mesasTamano\":" << rest.mesas.capacidadActual() << ",\"mesas\":[";
    for (int i = 0; i < rest.mesas.longitud(); i++) {           // Mesas (arreglo dinámico) con su comanda (pila)
        Mesa m = rest.mesas.obtener(i);
        double total = 0;
        string pila = jsonPila(rest.comandas[i], total);
        if (i > 0) o << ",";
        o << "{\"id\":" << m.id << ",\"capacidad\":" << m.capacidad << ",\"ocupada\":" << (m.ocupada ? "true" : "false")
          << ",\"mesero\":\"" << esc(nombreEmpleado(meseros, rest.meseroDeMesa[i])) << "\",\"comanda\":" << pila << ",\"total\":" << num(total) << "}";
    }
    double totalDom = 0;
    string pilaDom = jsonPila(rest.comandaDomicilio, totalDom);
    o << "],\"filaVip\":" << jsonFila(filas.vip) << ",\"filaComun\":" << jsonFila(filas.comun)
      << ",\"vipSeguidos\":" << filas.vipSeguidos << ",\"vipPorComun\":" << VIP_POR_COMUN
      << ",\"capacidadMaxima\":" << capacidadMaxima(rest)
      << ",\"meseros\":" << jsonGrupo(meseros) << ",\"domiciliarios\":" << jsonGrupo(domiciliarios)
      << ",\"domicilio\":{\"productos\":" << pilaDom << ",\"total\":" << num(totalDom) << "}"
      << ",\"ventas\":{\"total\":" << num(rest.ventasTotal) << ",\"cuentas\":" << rest.cuentasSalon << ",\"domicilios\":" << rest.pedidosDomicilio << "}}";
    return o.str();
}
