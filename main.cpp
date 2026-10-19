// SIMULADOR DE CAFETERÍA - Fase 1
// Archivo principal: menú e interfaz de consola (Persona 3), donde se unen las partes del grupo:
//   Persona 1 → arreglos dinámicos (menú y mesas) y pila (comanda de cada mesa)   [Proyecto1ED.h]
//   Persona 2 → fila de espera común (cola normal) y fila VIP (cola con prioridad) [Cliente.h, ColaComun.h, ColaVIP.h]
//   Persona 3 → turnos de meseros y domiciliarios (cola circular) y este menú     [ColaCircular.h, GrupoPersonal.h/.cpp]

#include <iostream>           // Para escribir en pantalla (std::cout) y leer del teclado (std::cin)
#include <iomanip>            // Para dar formato a los números (sin decimales en los precios)
#include <string>             // Para usar texto (std::string)
#include <cstdlib>            // Para exit(), que cierra el programa
#include <cctype>             // Para toupper(), que convierte una letra a mayúscula
#include "MesasMenu.h"        // Persona 1: Plato, Mesa, Pedidos, ArregloMenu, ArregloMesa, PilaPedidos
#include "Cliente.h"          // Persona 2: clientes y tipos de tarjeta
#include "ColaComun.h"        // Persona 2: fila de espera común (cola normal)
#include "ColaVIP.h"          // Persona 2: fila de espera VIP (cola con prioridad)
#include "GrupoPersonal.h"    // Persona 3: grupos de meseros y domiciliarios (cola circular)

#ifdef _WIN32                 // Esta parte solo se usa si el programa se compila en Windows
#include <windows.h>          // Para poder mostrar bien las tildes y la ñ en la consola de Windows
#endif

using namespace std;          // Permite escribir cout en lugar de std::cout

const int MAX_MESAS = 20;     // Cantidad máxima de mesas (cada mesa tiene su propia pila de comanda)


// ---------------------------------------------------------------
// Datos del programa, agrupados para pasarlos juntos a las funciones
// ---------------------------------------------------------------

struct FilasEspera {          // Filas de espera de clientes (Persona 2)
    Cola comun;               // Fila común (cola normal): clientes sin tarjeta
    ColaPrioridad vip;        // Fila VIP (cola con prioridad): clientes con tarjeta
    int turno;                // Número de turno que se le dará al próximo cliente que llegue
    int vipSeguidos;          // Cuántos clientes VIP se han atendido seguidos desde el último cliente común

    FilasEspera() : turno(1), vipSeguidos(0) {}    // Constructor: el primer turno es el 1 y todavía no se ha atendido ningún VIP
};

struct Restaurante {          // Menú, mesas, comandas y ventas (Persona 1)
    ArregloMenu menu;                     // Arreglo dinámico con los platos
    ArregloMesa mesas;                    // Arreglo dinámico con las mesas
    PilaPedidos comandas[MAX_MESAS];      // Una pila de pedidos por mesa (la posición coincide con la de la mesa en el arreglo)
    int meseroDeMesa[MAX_MESAS];          // ID del mesero que atiende cada mesa (0 = ninguno)
    PilaPedidos comandaDomicilio;         // Pila del pedido a domicilio que se está tomando
    int siguienteIdPlato;                 // Número que tendrá el próximo plato nuevo
    int siguienteIdMesa;                  // Número que tendrá la próxima mesa nueva
    double ventasTotal;                   // Dinero vendido en el día
    int cuentasSalon;                     // Cuántas cuentas del salón se han pagado
    int pedidosDomicilio;                 // Cuántos pedidos a domicilio se han confirmado

    Restaurante() : siguienteIdPlato(101), siguienteIdMesa(1), ventasTotal(0), cuentasSalon(0), pedidosDomicilio(0) {   // Constructor: todo empieza en cero
        for (int i = 0; i < MAX_MESAS; i++) meseroDeMesa[i] = 0;    // Al inicio ninguna mesa tiene mesero
    }
};


// ---------------------------------------------------------------
// Funciones de apoyo para leer datos de forma segura
// ---------------------------------------------------------------

int leerEntero(const string& mensaje) {            // Pide un número entero y repite la pregunta si se escribe otra cosa
    int valor;                                     // Aquí se guardará el número leído
    while (true) {                                 // Se repite hasta que el usuario escriba un número válido
        cout << mensaje;                           // Muestra la pregunta
        if (cin >> valor) {                        // Si lo escrito es un número, se leyó bien
            cin.ignore(10000, '\n');               // Descarta el Enter que quedó pendiente
            return valor;                          // Devuelve el número
        }
        if (cin.eof()) exit(0);                    // Si se cerró la entrada de datos, termina el programa (evita un ciclo infinito)
        cin.clear();                               // Limpia el estado de error de cin
        cin.ignore(10000, '\n');                   // Descarta lo que se escribió mal (hasta el Enter)
        cout << "Entrada no válida. Escriba un número.\n";    // Avisa y vuelve a preguntar
    }
}

double leerDecimal(const string& mensaje) {        // Pide un número (puede tener decimales), por ejemplo un precio
    double valor;                                  // Aquí se guardará el número leído
    while (true) {                                 // Se repite hasta que el usuario escriba un número válido
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(10000, '\n');
            return valor;
        }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Entrada no válida. Escriba un número.\n";
    }
}

string leerTexto(const string& mensaje) {          // Pide un texto (puede tener espacios)
    string texto;                                  // Aquí se guardará lo escrito
    cout << mensaje;                               // Muestra la pregunta
    getline(cin, texto);                           // Lee la línea completa, con espacios
    if (cin.eof()) exit(0);                        // Si se cerró la entrada de datos, termina el programa
    return texto;                                  // Devuelve el texto
}

void titulo(const string& texto) {                 // Muestra un título con adornos para separar las pantallas
    cout << "\n==============================\n";  // Línea de adorno arriba
    cout << "  " << texto << "\n";                 // El título
    cout << "==============================\n";    // Línea de adorno abajo
}

string mayusculas(string texto) {                  // Convierte un texto a mayúsculas (para los títulos)
    for (char& c : texto) {                        // Recorre cada letra del texto
        c = toupper(static_cast<unsigned char>(c));    // La convierte a mayúscula
    }
    return texto;                                  // Devuelve el texto convertido
}


// ---------------------------------------------------------------
// Meseros y domiciliarios (Persona 3: colas circulares)
// ---------------------------------------------------------------

void asignarEmpleado(GrupoPersonal& grupo) {       // Asigna al siguiente empleado libre del grupo y muestra el resultado
    Empleado e;                                    // Aquí se recibirá el empleado elegido
    if (grupo.asignarSiguienteLibre(e)) {          // Si se encontró a alguien libre
        cout << "\nSe asignó a " << e.nombre << " (ID " << e.id << ").\n";
    } else {                                       // Si todos estaban ocupados
        cout << "\nTodos los " << grupo.getPlural() << " están ocupados. Intente más tarde.\n";
    }
}

void liberarEmpleado(GrupoPersonal& grupo) {       // Libera a un empleado (cuando termina de atender o entregar)
    grupo.mostrar();                               // Muestra la lista para que el usuario vea los ID
    int id = leerEntero("\nID del empleado a liberar: ");    // Pide el ID del empleado
    if (grupo.liberar(id)) {                       // Si el ID existía
        cout << "El empleado quedó libre.\n";
    } else {                                       // Si no existía
        cout << "No existe un empleado con ese ID.\n";
    }
}

void agregarEmpleado(GrupoPersonal& grupo) {       // Agrega un empleado nuevo al grupo
    string nombre = leerTexto("Nombre del nuevo " + grupo.getTipo() + ": ");    // Pide el nombre
    if (nombre.empty()) {                          // Si no escribió nada
        cout << "El nombre no puede estar vacío.\n";
    } else if (grupo.agregar(nombre)) {            // Si se pudo agregar
        cout << grupo.getTipo() << " agregado correctamente.\n";
    } else {                                       // Si el grupo ya estaba lleno
        cout << "No se puede agregar: el grupo está lleno.\n";
    }
}

void menuGrupo(GrupoPersonal& grupo) {             // Submenú para administrar un grupo completo (meseros o domiciliarios)
    int opcion;                                    // Opción elegida por el usuario
    do {                                           // Se repite hasta que elija 0 (volver)
        titulo("ADMINISTRAR " + mayusculas(grupo.getPlural()));    // Título de la pantalla
        cout << "1. Ver lista y turnos\n";
        cout << "2. Agregar " << grupo.getTipo() << "\n";
        cout << "3. Asignar al siguiente libre\n";
        cout << "4. Liberar\n";
        cout << "0. Volver\n";
        opcion = leerEntero("Opción: ");           // Lee la opción
        switch (opcion) {                          // Según la opción elegida
            case 1: grupo.mostrar(); break;        // Muestra la lista con los turnos
            case 2: agregarEmpleado(grupo); break; // Agrega un empleado nuevo
            case 3: asignarEmpleado(grupo); break; // Asigna al siguiente libre
            case 4: liberarEmpleado(grupo); break; // Libera a un empleado
            case 0: break;                         // Vuelve al menú anterior
            default: cout << "Opción no válida.\n";    // Cualquier otro número
        }
    } while (opcion != 0);
}


// ---------------------------------------------------------------
// Menú de productos y mesas (Persona 1: arreglos dinámicos)
// ---------------------------------------------------------------

void mostrarMenuProductos(Restaurante& r) {        // Muestra todos los platos del menú
    cout << "\n--- MENÚ ---\n";
    if (r.menu.longitud() == 0) {                  // Si no hay platos
        cout << "El menú está vacío.\n";
        return;
    }
    for (int i = 0; i < r.menu.longitud(); i++) {  // Recorre todos los platos del arreglo
        Plato p = r.menu.obtener(i);               // Toma el plato de la posición i
        cout << "[" << i << "] " << p.nombre << " - $" << p.precio << "\n";    // El número entre corchetes es el que se escribe para pedirlo
    }
}

void mostrarMesas(Restaurante& r) {                // Muestra todas las mesas con su estado
    cout << "\n--- MESAS ---\n";
    for (int i = 0; i < r.mesas.longitud(); i++) { // Recorre todas las mesas del arreglo
        Mesa m = r.mesas.obtener(i);               // Toma la mesa de la posición i
        cout << "Mesa " << m.id << " (" << m.capacidad << " personas) - " << (m.ocupada ? "OCUPADA" : "LIBRE");
        if (m.ocupada) {                           // Si está ocupada, se muestran más datos
            cout << " - mesero ID " << r.meseroDeMesa[i] << " - productos en la comanda: " << r.comandas[i].cantidad();
        }
        cout << "\n";
    }
}

void agregarPlato(Restaurante& r) {                // Agrega un plato nuevo al menú
    string nombre = leerTexto("Nombre del plato: ");
    if (nombre.empty()) {                          // El nombre no puede estar vacío
        cout << "El nombre no puede estar vacío.\n";
        return;
    }
    double precio = leerDecimal("Precio: $");
    if (precio <= 0) {                             // El precio debe ser positivo
        cout << "El precio debe ser mayor a 0.\n";
        return;
    }
    r.menu.agregar(Plato(r.siguienteIdPlato, nombre, precio));    // Se agrega al arreglo dinámico
    r.siguienteIdPlato++;                          // El próximo plato tendrá el número siguiente
    cout << "Plato agregado al menú.\n";
}

void quitarUltimoPlato(Restaurante& r) {           // Quita el último plato del menú
    if (r.menu.longitud() == 0) {                  // Si no hay platos
        cout << "El menú está vacío.\n";
        return;
    }
    Plato ultimo = r.menu.obtener(r.menu.longitud() - 1);    // Guarda el último plato para mostrarlo
    r.menu.borrar();                               // Lo quita del arreglo dinámico
    cout << "Se quitó del menú: " << ultimo.nombre << "\n";
}

void buscarPlato(Restaurante& r) {                 // Busca un plato por su nombre exacto
    string nombre = leerTexto("Nombre del plato a buscar: ");
    int pos = r.menu.buscar(nombre);               // Devuelve la posición, o -1 si no existe
    if (pos == -1) {
        cout << "No existe ese plato en el menú.\n";
    } else {
        Plato p = r.menu.obtener(pos);
        cout << "Encontrado: [" << pos << "] " << p.nombre << " - $" << p.precio << "\n";
    }
}

void menuProductos(Restaurante& r) {               // Submenú para administrar el menú de productos
    int opcion;
    do {
        titulo("MENÚ DE PRODUCTOS");
        cout << "1. Ver menú\n";
        cout << "2. Agregar plato\n";
        cout << "3. Quitar el último plato\n";
        cout << "4. Buscar plato por nombre\n";
        cout << "0. Volver\n";
        opcion = leerEntero("Opción: ");
        switch (opcion) {
            case 1: mostrarMenuProductos(r); break;
            case 2: agregarPlato(r); break;
            case 3: quitarUltimoPlato(r); break;
            case 4: buscarPlato(r); break;
            case 0: break;
            default: cout << "Opción no válida.\n";
        }
    } while (opcion != 0);
}

void agregarMesa(Restaurante& r) {                 // Agrega una mesa nueva
    if (r.mesas.longitud() >= MAX_MESAS) {         // No se pueden pasar de la cantidad máxima
        cout << "Se alcanzó el máximo de " << MAX_MESAS << " mesas.\n";
        return;
    }
    int capacidad = leerEntero("Capacidad de la mesa (personas): ");
    if (capacidad <= 0) {                          // Debe caber al menos una persona
        cout << "La capacidad debe ser mayor a 0.\n";
        return;
    }
    r.mesas.agregar(Mesa(r.siguienteIdMesa, capacidad));    // Se agrega al arreglo dinámico
    cout << "Mesa " << r.siguienteIdMesa << " agregada.\n";
    r.siguienteIdMesa++;                           // La próxima mesa tendrá el número siguiente
}

void menuMesas(Restaurante& r) {                   // Submenú para administrar las mesas
    int opcion;
    do {
        titulo("MESAS");
        cout << "1. Ver mesas\n";
        cout << "2. Agregar mesa\n";
        cout << "0. Volver\n";
        opcion = leerEntero("Opción: ");
        switch (opcion) {
            case 1: mostrarMesas(r); break;
            case 2: agregarMesa(r); break;
            case 0: break;
            default: cout << "Opción no válida.\n";
        }
    } while (opcion != 0);
}


// ---------------------------------------------------------------
// Salón: fila de espera (Persona 2), mesas y comandas (Persona 1), meseros (Persona 3)
// ---------------------------------------------------------------

int capacidadMaxima(Restaurante& r) {              // Devuelve la capacidad de la mesa más grande
    int maxima = 0;
    for (int i = 0; i < r.mesas.longitud(); i++) {
        int c = r.mesas.obtener(i).capacidad;
        if (c > maxima) maxima = c;                // Se queda con la mayor
    }
    return maxima;
}

int buscarMesaLibre(Restaurante& r, int personas) {    // Busca la mesa libre más pequeña donde quepa el grupo (devuelve su posición, o -1)
    int mejor = -1;                                // Posición de la mejor mesa encontrada hasta ahora
    for (int i = 0; i < r.mesas.longitud(); i++) {
        Mesa m = r.mesas.obtener(i);
        if (!m.ocupada && m.capacidad >= personas) {           // Libre y con espacio suficiente
            if (mejor == -1 || m.capacidad < r.mesas.obtener(mejor).capacidad) {   // Y más pequeña que la mejor anterior
                mejor = i;
            }
        }
    }
    return mejor;
}

void registrarCliente(FilasEspera& filas, Restaurante& r, bool conTarjeta) {    // Registra la llegada de un cliente (con o sin tarjeta)
    string nombre = leerTexto("Nombre del cliente: ");          // Pide el nombre
    if (nombre.empty()) {                                       // Validación: el nombre no puede estar vacío
        cout << "El nombre no puede estar vacío.\n";
        return;                                                 // Cancela el registro
    }
    int personas = leerEntero("Cantidad de personas: ");        // Pide la cantidad de personas
    if (personas <= 0) {                                        // Validación: debe ser al menos 1
        cout << "La cantidad de personas debe ser mayor a 0.\n";
        return;
    }
    if (personas > capacidadMaxima(r)) {                        // Validación: debe existir alguna mesa donde quepan
        cout << "Ninguna mesa tiene capacidad para " << personas << " personas (máximo: " << capacidadMaxima(r) << ").\n";
        return;
    }
    int tarjeta = SIN_TARJETA;                                  // Por defecto, sin tarjeta
    if (conTarjeta) {                                           // Solo se pregunta la tarjeta si el cliente llega por la fila VIP
        cout << "Tipo de tarjeta:\n";
        cout << "  1. Cliente frecuente\n";
        cout << "  2. Amigo del gerente\n";
        cout << "  3. Persona con prestigio\n";
        tarjeta = leerEntero("Tarjeta: ");                      // Pide el tipo de tarjeta
        if (tarjeta < CLIENTE_FRECUENTE || tarjeta > PRESTIGIO) {    // Debe estar entre 1 y 3
            cout << "Tarjeta no válida.\n";
            return;
        }
    }
    Cliente c(nombre, personas, tarjeta, filas.turno);          // Crea el cliente con los datos leídos
    filas.turno++;                                              // El siguiente cliente tendrá el turno siguiente
    if (c.esvip()) {                                            // Con tarjeta: va a la fila VIP (cola con prioridad)
        filas.vip.encolar(c);
        cout << "Agregado a la fila VIP: " << c.convertir() << "\n";
    } else {                                                    // Sin tarjeta: va a la fila común (cola normal)
        filas.comun.encolar(c);
        cout << "Agregado a la fila común: " << c.convertir() << "\n";
    }
}

void verFilas(FilasEspera& filas) {                             // Muestra las dos filas de espera
    cout << "\nFila VIP (" << filas.vip.longitud() << "):\n";   // Título con la cantidad de clientes VIP
    filas.vip.mostrar();                                        // Lista de la fila VIP
    cout << "Fila común (" << filas.comun.longitud() << "):\n"; // Título con la cantidad de clientes comunes
    filas.comun.mostrar();                                      // Lista de la fila común
}

bool siguienteCliente(FilasEspera& filas, Cliente& c, bool& deVip) {    // Averigua quién sigue SIN sacarlo de la fila
    // Regla de la fila: después de 2 clientes VIP seguidos le toca a la fila común. Si una de las filas está vacía, se atiende la otra.
    if (filas.vipSeguidos >= VIP_POR_COMUN && filas.comun.frente(c)) {    // Ya van 2 VIP seguidos y hay alguien en la fila común
        deVip = false;
        return true;
    }
    if (filas.vip.frente(c)) {                                  // Si no, le toca al primero de la fila VIP
        deVip = true;
        return true;
    }
    if (filas.comun.frente(c)) {                                // Si no hay VIP, le toca al primero de la fila común
        deVip = false;
        return true;
    }
    return false;                                               // Ambas filas están vacías
}

void llamarSiguiente(FilasEspera& filas, Restaurante& r, GrupoPersonal& meseros) {    // Llama al siguiente cliente, le da mesa y le asigna mesero
    Cliente c;                                                  // Aquí se recibirá el cliente que sigue
    bool deVip = false;                                         // Indica si viene de la fila VIP
    if (!siguienteCliente(filas, c, deVip)) {                   // Primero se mira quién sigue, sin sacarlo
        cout << "\nNo hay clientes esperando.\n";
        return;
    }
    if (meseros.cantidadLibres() == 0) {                        // Si no hay meseros libres, el cliente sigue esperando
        cout << "\nNo hay meseros libres. Libere uno antes de llamar a un cliente.\n";
        return;
    }
    int pos = buscarMesaLibre(r, c.personas);                   // Se busca una mesa libre donde quepa el grupo
    if (pos == -1) {                                            // Si no hay, el cliente sigue esperando
        cout << "\nNo hay una mesa libre para " << c.personas << " persona(s). El cliente sigue esperando.\n";
        return;
    }
    Cliente salida;                                             // Aquí se recibe al cliente al sacarlo de la fila
    if (deVip) {                                                // Ahora sí se saca de su fila y se actualiza la cuenta de VIP seguidos
        filas.vip.desencolar(salida);
        filas.vipSeguidos++;                                    // Un VIP más seguido
    } else {
        filas.comun.desencolar(salida);
        filas.vipSeguidos = 0;                                  // Se reinicia la cuenta
    }
    Empleado mesero;                                            // Aquí se recibirá el mesero elegido
    meseros.asignarSiguienteLibre(mesero);                      // Cola circular: se asigna al siguiente mesero libre
    r.mesas.cambiarEstado(pos, true);                           // La mesa pasa a estar ocupada
    r.meseroDeMesa[pos] = mesero.id;                            // Se anota qué mesero atiende esa mesa
    cout << "\nPasa a mesa" << (deVip ? " (VIP)" : "") << ": " << salida.convertir() << "\n";
    cout << "Mesa asignada: " << r.mesas.obtener(pos).id << " - Mesero: " << mesero.nombre << " (ID " << mesero.id << ")\n";
}

int pedirMesaOcupada(Restaurante& r) {             // Pide el número de una mesa y devuelve su posición si existe y está ocupada (si no, -1)
    mostrarMesas(r);                               // Muestra las mesas para que el usuario elija
    int id = leerEntero("Número de la mesa: ");
    int pos = r.mesas.buscar(id);                  // Busca la mesa en el arreglo
    if (pos == -1) {
        cout << "No existe esa mesa.\n";
        return -1;
    }
    if (!r.mesas.obtener(pos).ocupada) {
        cout << "Esa mesa no está ocupada.\n";
        return -1;
    }
    return pos;
}

void tomarComanda(Restaurante& r) {                // Agrega un producto a la comanda (pila) de una mesa
    int pos = pedirMesaOcupada(r);
    if (pos == -1) return;                         // Si la mesa no sirve, se cancela
    mostrarMenuProductos(r);
    int indice = leerEntero("Número del plato a pedir: ");
    if (indice < 0 || indice >= r.menu.longitud()) {    // Debe ser un plato que exista
        cout << "Plato no válido.\n";
        return;
    }
    Plato p = r.menu.obtener(indice);
    r.comandas[pos].agregar(Pedidos(p, r.mesas.obtener(pos).id));    // Se APILA el pedido en la comanda de esa mesa
    cout << "[+] Agregado: " << p.nombre << ". Productos en la comanda: " << r.comandas[pos].cantidad() << "\n";
}

void deshacerProducto(Restaurante& r) {            // Quita el último producto de la comanda de una mesa
    int pos = pedirMesaOcupada(r);
    if (pos == -1) return;
    if (r.comandas[pos].estaVacia()) {             // Si no hay nada que deshacer
        cout << "Esa mesa no tiene productos en la comanda.\n";
        return;
    }
    string nombre = r.comandas[pos].top().platoPedido.nombre;    // Mira el producto del tope de la pila
    r.comandas[pos].borrar();                      // Lo DESAPILA: el último en entrar es el primero en salir
    cout << "[!] Se deshizo: " << nombre << "\n";
}

void cerrarCuenta(Restaurante& r, GrupoPersonal& meseros) {    // Cobra la comanda de una mesa y la libera
    int pos = pedirMesaOcupada(r);
    if (pos == -1) return;
    double total = 0;                              // Total a pagar
    cout << "\n--- CUENTA MESA " << r.mesas.obtener(pos).id << " (del último producto al primero) ---\n";
    while (!r.comandas[pos].estaVacia()) {         // Se desapila toda la comanda, producto por producto
        Plato p = r.comandas[pos].top().platoPedido;
        cout << "  " << p.nombre << " - $" << p.precio << "\n";
        total += p.precio;                         // Se suma al total
        r.comandas[pos].borrar();                  // Se quita del tope
    }
    cout << "TOTAL: $" << total << "\n";
    r.ventasTotal += total;                        // Se suma a las ventas del día
    r.cuentasSalon++;
    if (r.meseroDeMesa[pos] != 0) {                // Si la mesa tenía mesero, queda libre
        meseros.liberar(r.meseroDeMesa[pos]);
        r.meseroDeMesa[pos] = 0;
    }
    r.mesas.cambiarEstado(pos, false);             // La mesa queda libre
    cout << "Mesa liberada.\n";
}

void menuSalon(GrupoPersonal& meseros, FilasEspera& filas, Restaurante& r) {    // Menú del salón
    int opcion;                                                 // Opción elegida por el usuario
    do {                                                        // Se repite hasta que elija 0 (volver)
        titulo("SALÓN");
        cout << "1. Llega un cliente (fila común)\n";
        cout << "2. Llega un cliente con tarjeta (fila VIP)\n";
        cout << "3. Llamar al siguiente cliente (asigna mesa y mesero)\n";
        cout << "4. Ver filas de espera\n";
        cout << "5. Ver mesas\n";
        cout << "6. Tomar comanda (agregar producto)\n";
        cout << "7. Deshacer último producto\n";
        cout << "8. Pagar y cerrar cuenta\n";
        cout << "0. Volver\n";
        opcion = leerEntero("Opción: ");
        switch (opcion) {
            case 1: registrarCliente(filas, r, false); break;   // Persona 2: cola normal
            case 2: registrarCliente(filas, r, true); break;    // Persona 2: cola con prioridad
            case 3: llamarSiguiente(filas, r, meseros); break;  // Personas 1, 2 y 3 trabajando juntas
            case 4: verFilas(filas); break;
            case 5: mostrarMesas(r); break;
            case 6: tomarComanda(r); break;                     // Persona 1: pila
            case 7: deshacerProducto(r); break;                 // Persona 1: pila
            case 8: cerrarCuenta(r, meseros); break;
            case 0: break;
            default: cout << "Opción no válida.\n";
        }
    } while (opcion != 0);
}


// ---------------------------------------------------------------
// Domicilios (pila de la Persona 1 + cola circular de domiciliarios de la Persona 3)
// ---------------------------------------------------------------

void agregarProductoDomicilio(Restaurante& r) {    // Agrega un producto al pedido a domicilio que se está tomando
    mostrarMenuProductos(r);
    int indice = leerEntero("Número del plato a pedir: ");
    if (indice < 0 || indice >= r.menu.longitud()) {
        cout << "Plato no válido.\n";
        return;
    }
    Plato p = r.menu.obtener(indice);
    r.comandaDomicilio.agregar(Pedidos(p, 0));     // Se apila (mesa 0 = no es de ninguna mesa)
    cout << "[+] Agregado: " << p.nombre << ". Productos en el pedido: " << r.comandaDomicilio.cantidad() << "\n";
}

void deshacerProductoDomicilio(Restaurante& r) {   // Quita el último producto del pedido a domicilio
    if (r.comandaDomicilio.estaVacia()) {
        cout << "El pedido a domicilio no tiene productos.\n";
        return;
    }
    string nombre = r.comandaDomicilio.top().platoPedido.nombre;
    r.comandaDomicilio.borrar();                   // Se desapila el último producto
    cout << "[!] Se deshizo: " << nombre << "\n";
}

void confirmarDomicilio(Restaurante& r, GrupoPersonal& domiciliarios) {    // Confirma el pedido, lo cobra y asigna un domiciliario
    if (r.comandaDomicilio.estaVacia()) {
        cout << "El pedido a domicilio no tiene productos.\n";
        return;
    }
    if (domiciliarios.cantidadLibres() == 0) {     // Si no hay domiciliarios libres, el pedido queda pendiente
        cout << "No hay domiciliarios libres. El pedido sigue abierto; libere uno e intente de nuevo.\n";
        return;
    }
    string cliente = leerTexto("Nombre del cliente: ");
    string direccion = leerTexto("Dirección de entrega: ");
    double total = 0;
    cout << "\n--- PEDIDO A DOMICILIO (del último producto al primero) ---\n";
    while (!r.comandaDomicilio.estaVacia()) {      // Se desapila todo el pedido
        Plato p = r.comandaDomicilio.top().platoPedido;
        cout << "  " << p.nombre << " - $" << p.precio << "\n";
        total += p.precio;
        r.comandaDomicilio.borrar();
    }
    cout << "TOTAL: $" << total << "\n";
    r.ventasTotal += total;                        // Se suma a las ventas del día
    r.pedidosDomicilio++;
    Empleado domiciliario;
    domiciliarios.asignarSiguienteLibre(domiciliario);    // Cola circular: se asigna al siguiente domiciliario libre
    cout << "Entrega para " << cliente << " en " << direccion << ".\n";
    cout << "Domiciliario: " << domiciliario.nombre << " (ID " << domiciliario.id << ")\n";
}

void menuDomicilios(GrupoPersonal& domiciliarios, Restaurante& r) {    // Menú de domicilios
    int opcion;
    do {
        titulo("DOMICILIOS");
        cout << "1. Agregar producto al pedido\n";
        cout << "2. Deshacer último producto\n";
        cout << "3. Confirmar pedido y asignar domiciliario\n";
        cout << "4. Liberar domiciliario (ya entregó)\n";
        cout << "0. Volver\n";
        opcion = leerEntero("Opción: ");
        switch (opcion) {
            case 1: agregarProductoDomicilio(r); break;
            case 2: deshacerProductoDomicilio(r); break;
            case 3: confirmarDomicilio(r, domiciliarios); break;
            case 4: liberarEmpleado(domiciliarios); break;
            case 0: break;
            default: cout << "Opción no válida.\n";
        }
    } while (opcion != 0);
}


// ---------------------------------------------------------------
// Administración, ventas y programa principal
// ---------------------------------------------------------------

void menuAdministracion(GrupoPersonal& meseros, GrupoPersonal& domiciliarios, Restaurante& r) {    // Menú de administración
    int opcion;
    do {
        titulo("ADMINISTRACIÓN");
        cout << "1. Meseros\n";
        cout << "2. Domiciliarios\n";
        cout << "3. Menú de productos\n";
        cout << "4. Mesas\n";
        cout << "0. Volver\n";
        opcion = leerEntero("Opción: ");
        switch (opcion) {
            case 1: menuGrupo(meseros); break;                  // Persona 3: administrar meseros
            case 2: menuGrupo(domiciliarios); break;            // Persona 3: administrar domiciliarios
            case 3: menuProductos(r); break;                    // Persona 1: arreglo dinámico del menú
            case 4: menuMesas(r); break;                        // Persona 1: arreglo dinámico de mesas
            case 0: break;
            default: cout << "Opción no válida.\n";
        }
    } while (opcion != 0);
}

void verVentas(Restaurante& r) {                   // Muestra el resumen de ventas del día
    titulo("VENTAS DEL DÍA");
    cout << "Cuentas del salón pagadas: " << r.cuentasSalon << "\n";
    cout << "Pedidos a domicilio: " << r.pedidosDomicilio << "\n";
    cout << "Total vendido: $" << r.ventasTotal << "\n";
}

int main() {
#ifdef _WIN32                                  // Solo se ejecuta en Windows
    SetConsoleOutputCP(CP_UTF8);               // Configura la consola para mostrar tildes y ñ
#endif
    cout << fixed << setprecision(0);          // Los precios se muestran sin decimales (por ejemplo 25000)

    GrupoPersonal meseros("Mesero", "Meseros", 6);                      // Cola circular de meseros (capacidad máxima: 6)
    GrupoPersonal domiciliarios("Domiciliario", "Domiciliarios", 4);    // Cola circular de domiciliarios (capacidad máxima: 4)
    FilasEspera filas;                                                  // Filas de espera de clientes (cola normal y cola con prioridad)
    Restaurante rest;                                                   // Menú, mesas, comandas y ventas

    meseros.agregar("Carlos");                 // Datos de ejemplo para poder probar el programa desde el inicio
    meseros.agregar("Laura");
    meseros.agregar("Andrés");
    domiciliarios.agregar("Mateo");
    domiciliarios.agregar("Sofía");

    rest.menu.agregar(Plato(101, "Hamburguesa Artesanal", 25000));      // Menú inicial (el de la Persona 1)
    rest.menu.agregar(Plato(102, "Alitas BBQ (x10)", 18000));
    rest.menu.agregar(Plato(103, "Limonada de Coco", 8000));
    rest.menu.agregar(Plato(104, "Helado de Vainilla", 5000));
    rest.siguienteIdPlato = 105;               // El próximo plato nuevo será el 105

    rest.mesas.agregar(Mesa(1, 2));            // Mesas iniciales (las de la Persona 1)
    rest.mesas.agregar(Mesa(2, 4));
    rest.mesas.agregar(Mesa(3, 6));
    rest.siguienteIdMesa = 4;                  // La próxima mesa nueva será la 4

    int opcion;                                // Opción elegida en el menú principal
    do {                                       // El menú se repite hasta que se elija 0
        titulo("CAFETERÍA");
        cout << "1. Salón\n";
        cout << "2. Domicilios\n";
        cout << "3. Administración\n";
        cout << "4. Ventas del día\n";
        cout << "0. Salir\n";
        opcion = leerEntero("Opción: ");
        switch (opcion) {
            case 1: menuSalon(meseros, filas, rest); break;
            case 2: menuDomicilios(domiciliarios, rest); break;
            case 3: menuAdministracion(meseros, domiciliarios, rest); break;
            case 4: verVentas(rest); break;
            case 0: cout << "\n¡Hasta pronto!\n"; break;
            default: cout << "Opción no válida.\n";
        }
    } while (opcion != 0);

    return 0;                                  // 0 indica que el programa terminó sin errores
}
