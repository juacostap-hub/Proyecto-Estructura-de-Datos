#ifndef COLACIRCULAR_H    // Evita que este archivo se lea más de una vez al compilar
#define COLACIRCULAR_H    // Marca que este archivo ya fue leído

#include "Empleado.h"     // Es necesario porque la cola guarda objetos de tipo Empleado


// COLA CIRCULAR de empleados (hecha a mano con un arreglo de tamaño fijo)
// Es una cola FIFO: el primero que entra es el primero que sale.
// La diferencia con una cola normal: tiene un tamaño máximo, y cuando "final" llega
// a la última posición del arreglo, vuelve a la posición 0 (por eso es circular).
// Así se reutilizan los espacios que quedaron libres al sacar elementos.
//
//   frente   → posición del empleado que sigue en el turno (el que sale primero)
//   final    → posición donde se guardará el próximo empleado que entre
//   cantidad → cuántos empleados hay guardados ahora mismo
//
// Para dar la vuelta se usa el operador módulo (%):   (posicion + 1) % capacidad


class ColaCircular {
  private:
    Empleado* datos;      // Arreglo donde se guardan los empleados (se crea con "new")
    int capacidad;        // Tamaño máximo de la cola (no cambia)
    int frente;           // Índice del primer elemento (el próximo en salir)
    int final;            // Índice donde entrará el próximo elemento
    int cantidad;         // Cantidad actual de elementos guardados

  public:

    explicit ColaCircular(int capacidad = 10)                         // Constructor: crea una cola vacía con la capacidad indicada
        : capacidad(capacidad), frente(0), final(0), cantidad(0) {    // Empieza vacía: frente y final en 0, sin elementos
        datos = new Empleado[capacidad];                              // Pide memoria para el arreglo con "new"
    }

    ~ColaCircular() {         // Destructor: se ejecuta solo cuando la cola deja de existir
        delete[] datos;       // Libera la memoria del arreglo (así no queda memoria perdida)
    }

    ColaCircular(const ColaCircular&) = delete;                // Se prohíbe copiar la cola (una copia compartiría el mismo arreglo y daría errores)
    ColaCircular& operator=(const ColaCircular&) = delete;     // Tampoco se permite asignar una cola a otra

    bool estaVacia() const { return cantidad == 0; }           // Dice si no hay ningún elemento
    bool estaLlena() const { return cantidad == capacidad; }   // Dice si ya no cabe ningún elemento más
    int tamano() const { return cantidad; }                    // Devuelve cuántos elementos hay ahora
    int getCapacidad() const { return capacidad; }             // Devuelve el tamaño máximo de la cola
    int getFrente() const { return frente; }                   // AGREGADO para la interfaz gráfica: posición del arreglo donde está el frente
    int getFinal() const { return final; }                     // AGREGADO para la interfaz gráfica: posición del arreglo donde entrará el próximo
    const Empleado& ranura(int k) const { return datos[k]; }   // AGREGADO para la interfaz gráfica: contenido de la posición k del arreglo (sin importar el turno)

    bool encolar(const Empleado& e) {       // Agrega un empleado al final de la cola
        if (estaLlena()) return false;      // Retorna false si no hay espacio
        datos[final] = e;                   // Guarda el empleado en la posición "final"
        final = (final + 1) % capacidad;    // Avanza "final"; si pasa de la última posición, vuelve a 0 (aquí está lo circular)
        cantidad++;                         // Ahora hay un elemento más
        return true;                        // Se agregó con éxito
    }

    bool desencolar(Empleado& salida) {     // Saca al empleado del frente y lo devuelve en "salida"
        if (estaVacia()) return false;      // Retorna false si no hay a quién sacar
        salida = datos[frente];             // Entrega el empleado por la referencia
        frente = (frente + 1) % capacidad;  // Avanza "frente"; si pasa de la última posición, vuelve a 0
        cantidad--;                         // Ahora hay un elemento menos
        return true;                        // Se sacó con éxito
    }

    const Empleado* obtener(int i) const {                  // Consulta al empleado en la posición i SIN sacarlo (i = 0 es el del frente)
        if (i < 0 || i >= cantidad) return nullptr;         // Si la posición no existe, no devuelve nada (nullptr)
        return &datos[(frente + i) % capacidad];            // Calcula la posición real en el arreglo (dando la vuelta si hace falta)
    }
};

#endif                    // Fin de la protección contra lecturas repetidas
