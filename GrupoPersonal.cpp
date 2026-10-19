#include "GrupoPersonal.h"   // Es necesario porque aquí se escribe el código de la clase GrupoPersonal
#include <iostream>          // Para escribir en pantalla (std::cout)

using namespace std;         // Permite escribir cout en lugar de std::cout


GrupoPersonal::GrupoPersonal(const string& tipo, const string& plural, int capacidad)   // Constructor del grupo
    : tipo(tipo), plural(plural), cola(capacidad), siguienteId(1) {}                    // Guarda los nombres, crea la cola con esa capacidad y empieza a numerar desde 1


bool GrupoPersonal::agregar(const string& nombre) {            // Agrega un empleado nuevo al final de los turnos
    if (cola.estaLlena()) return false;                        // Si ya no cabe nadie más, retorna false
    cola.encolar(Empleado(siguienteId, nombre));               // Crea el empleado con su id y su nombre y lo mete a la cola
    siguienteId++;                                             // El próximo empleado tendrá el número siguiente
    return true;                                               // Se agregó con éxito
}


bool GrupoPersonal::asignarSiguienteLibre(Empleado& asignado) {   // Busca al primer empleado libre según el turno
    int n = cola.tamano();                    // Guarda cuántos empleados hay (se revisa a cada uno como máximo una vez)
    for (int i = 0; i < n; i++) {             // Repite como máximo n veces
        Empleado e;                           // Aquí se recibirá el empleado que salga del frente
        cola.desencolar(e);                   // Saca al empleado del frente de la cola
        if (!e.ocupado) {                     // Si está libre, es el elegido
            e.ocupado = true;                 // Pasa a estar ocupado
            e.servicios++;                    // Se suma un servicio más a su cuenta
            cola.encolar(e);                  // Vuelve a entrar, pero al FINAL de la cola (así se turnan)
            asignado = e;                     // Entrega el empleado elegido por la referencia
            return true;                      // Se encontró a alguien libre
        }
        cola.encolar(e);                      // Si estaba ocupado, vuelve al final y se prueba con el siguiente
    }
    return false;                             // Se revisó a todos y ninguno estaba libre
}


bool GrupoPersonal::liberar(int id) {         // Marca como libre al empleado que tenga ese id
    bool encontrado = false;                  // Empieza en false: todavía no se ha encontrado
    int n = cola.tamano();                    // Cuántos empleados hay
    for (int i = 0; i < n; i++) {             // Se da una vuelta completa a la cola (así queda en el mismo orden)
        Empleado e;                           // Aquí se recibirá el empleado que salga del frente
        cola.desencolar(e);                   // Saca al empleado del frente
        if (e.id == id) {                     // Si es el empleado buscado
            e.ocupado = false;                // Queda libre otra vez
            encontrado = true;                // Se anota que sí se encontró
        }
        cola.encolar(e);                      // Vuelve a entrar al final (con su estado actualizado)
    }
    return encontrado;                        // true si existía un empleado con ese id, false si no
}


void GrupoPersonal::mostrar() const {         // Imprime el estado de todos los empleados (el primero es el siguiente en turno)
    cout << "\n--- " << plural << " (el primero de la lista es el siguiente en turno) ---\n";   // Título de la lista
    if (cola.estaVacia()) {                   // Si no hay nadie registrado
        cout << "No hay " << plural << " registrados.\n";    // Avisa y termina
        return;
    }
    for (int i = 0; i < cola.tamano(); i++) {                // Recorre desde el frente hasta el final, sin sacar a nadie
        const Empleado* e = cola.obtener(i);                 // Consulta al empleado en la posición i
        cout << i + 1 << ". [ID " << e->id << "] " << e->nombre                  // Imprime "posición. [ID] nombre"
             << " - " << (e->ocupado ? "OCUPADO" : "LIBRE")                      // Imprime su estado
             << " - servicios: " << e->servicios << "\n";                        // Imprime cuántas veces ha trabajado
    }
    cout << "Libres: " << cantidadLibres() << " de " << total()                  // Resumen: cuántos libres hay del total
         << " (capacidad máxima: " << cola.getCapacidad() << ")\n";              // y cuál es la capacidad máxima
}


int GrupoPersonal::total() const {            // Cuántos empleados hay en el grupo
    return cola.tamano();                     // Es la cantidad de elementos de la cola
}


int GrupoPersonal::cantidadLibres() const {   // Cuenta cuántos empleados están libres
    int libres = 0;                           // Contador
    for (int i = 0; i < cola.tamano(); i++) { // Recorre todos los empleados sin sacarlos
        if (!cola.obtener(i)->ocupado) libres++;     // Si el empleado no está ocupado, se cuenta
    }
    return libres;                            // Devuelve el total de libres
}
