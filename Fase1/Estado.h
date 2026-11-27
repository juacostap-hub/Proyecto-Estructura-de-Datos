#ifndef ESTADO_H          // Evita que este archivo se lea más de una vez al compilar
#define ESTADO_H          // Marca que este archivo ya fue leído

#include "Proyecto1ED.h"      // Persona 1: Plato, Mesa, Pedidos, ArregloMenu, ArregloMesa, PilaPedidos
#include "Cliente.h"          // Persona 2: clientes y tipos de tarjeta
#include "ColaComun.h"        // Persona 2: fila de espera común (cola normal)
#include "ColaVIP.h"          // Persona 2: fila de espera VIP (cola con prioridad)
#include "GrupoPersonal.h"    // Persona 3: grupos de meseros y domiciliarios (cola circular)

// DATOS Y REGLAS COMPARTIDAS
// Las usan las dos interfaces del programa: la de consola (Consola.cpp) y la gráfica (Cafeteria.cpp y Servidor.cpp).

const int MAX_MESAS = 20;     // Cantidad máxima de mesas (cada mesa tiene su propia pila de comanda)

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

inline void cargarDatosIniciales(GrupoPersonal& meseros, GrupoPersonal& domiciliarios, Restaurante& rest) {    // Datos de ejemplo para poder probar el programa desde el inicio
    meseros.agregar("Carlos");
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
}

inline int capacidadMaxima(Restaurante& r) {       // Devuelve la capacidad de la mesa más grande
    int maxima = 0;
    for (int i = 0; i < r.mesas.longitud(); i++) {
        int c = r.mesas.obtener(i).capacidad;
        if (c > maxima) maxima = c;                // Se queda con la mayor
    }
    return maxima;
}

inline int buscarMesaLibre(Restaurante& r, int personas) {    // Busca la mesa libre más pequeña donde quepa el grupo (devuelve su posición, o -1)
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

inline bool siguienteCliente(FilasEspera& filas, Cliente& c, bool& deVip) {    // Averigua quién sigue SIN sacarlo de la fila
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

#endif                    // Fin de la protección contra lecturas repetidas
