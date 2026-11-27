
// ARREGLO DINÁMICO

#ifndef ARREGLO_DINAMICO_H
#define ARREGLO_DINAMICO_H

#include <stdexcept>

using namespace std;

template <typename T>
class ArregloDinamico {
private:
    T* datos;
    int tamanoActual;
    int capacidad;
    const double AUREO = 1.6180339;
    const int CAPACIDAD_MINIMA = 10;

    void redimensionar(int nuevaCapacidad) {
        if (nuevaCapacidad < CAPACIDAD_MINIMA) {
            nuevaCapacidad = CAPACIDAD_MINIMA;
        }

        T* nuevoBloque = new T[nuevaCapacidad];
        for (int i = 0; i < tamanoActual; ++i) {
            nuevoBloque[i] = datos[i];
        }

        delete[] datos;
        datos = nuevoBloque;
        capacidad = nuevaCapacidad;
    }

    // Reduce memoria al eliminar elementos manteniendo el piso de 10
    void verificarContraccion() {
        if (tamanoActual == 0 && capacidad > CAPACIDAD_MINIMA) {
            redimensionar(CAPACIDAD_MINIMA);
            return;
        }
        if (tamanoActual > 0 && (tamanoActual * AUREO * AUREO) < capacidad && capacidad > CAPACIDAD_MINIMA) {
            int nuevaCapacidad = static_cast<int>(tamanoActual * AUREO);
            if (nuevaCapacidad < CAPACIDAD_MINIMA) {
                nuevaCapacidad = CAPACIDAD_MINIMA;
            }
            if (nuevaCapacidad < capacidad) {
                redimensionar(nuevaCapacidad);
            }
        }
    }

public:
    // Inicia con capacidad 10 según la guía del proyecto
    ArregloDinamico(int capacidadInicial = 10) {
        capacidad = (capacidadInicial >= CAPACIDAD_MINIMA) ? capacidadInicial : CAPACIDAD_MINIMA;
        tamanoActual = 0;
        datos = new T[capacidad];
    }

    // Constructor de copia (Regla de Tres)
    ArregloDinamico(const ArregloDinamico<T>& otro) {
        capacidad = otro.capacidad;
        tamanoActual = otro.tamanoActual;
        datos = new T[capacidad];
        for (int i = 0; i < tamanoActual; ++i) {
            datos[i] = otro.datos[i];
        }
    }

    // Operador de asignación (Regla de Tres)
    ArregloDinamico<T>& operator=(const ArregloDinamico<T>& otro) {
        if (this == &otro) {
            return *this;
        }

        delete[] datos;

        capacidad = otro.capacidad;
        tamanoActual = otro.tamanoActual;
        datos = new T[capacidad];
        for (int i = 0; i < tamanoActual; ++i) {
            datos[i] = otro.datos[i];
        }

        return *this;
    }

    ~ArregloDinamico() {
        delete[] datos;
        datos = nullptr;
    }

    // Inserta al final en O(1) amortizado creciendo x 1.618
    void agregar(const T& elemento) {
        if (tamanoActual == capacidad) {
            int nuevaCapacidad = static_cast<int>(capacidad * AUREO);
            if (nuevaCapacidad <= capacidad) {
                nuevaCapacidad = capacidad + 1;
            }
            redimensionar(nuevaCapacidad);
        }
        datos[tamanoActual++] = elemento;
    }

    bool insertarEn(int indice, const T& elemento) {
        if (indice < 0 || indice > tamanoActual) {
            return false;
        }
        if (tamanoActual == capacidad) {
            int nuevaCapacidad = static_cast<int>(capacidad * AUREO);
            if (nuevaCapacidad <= capacidad) {
                nuevaCapacidad = capacidad + 1;
            }
            redimensionar(nuevaCapacidad);
        }
        for (int i = tamanoActual; i > indice; --i) {
            datos[i] = datos[i - 1];
        }
        datos[indice] = elemento;
        ++tamanoActual;
        return true;
    }

    bool eliminarEn(int indice) {
        if (indice < 0 || indice >= tamanoActual) {
            return false;
        }
        for (int i = indice; i < tamanoActual - 1; ++i) {
            datos[i] = datos[i + 1];
        }
        --tamanoActual;
        verificarContraccion();
        return true;
    }

    // Útil para comportamiento de pila (LIFO) y borrado al final
    bool eliminarUltimo() {
        if (tamanoActual <= 0) {
            return false;
        }
        --tamanoActual;
        verificarContraccion();
        return true;
    }

    T& obtener(int indice) {
        if (indice < 0 || indice >= tamanoActual) {
            throw out_of_range("Indice fuera de rango en ArregloDinamico");
        }
        return datos[indice];
    }

    const T& obtener(int indice) const {
        if (indice < 0 || indice >= tamanoActual) {
            throw out_of_range("Indice fuera de rango en ArregloDinamico");
        }
        return datos[indice];
    }

    T& ultimo() {
        if (tamanoActual <= 0) {
            throw out_of_range("ArregloDinamico vacio");
        }
        return datos[tamanoActual - 1];
    }

    const T& ultimo() const {
        if (tamanoActual <= 0) {
            throw out_of_range("ArregloDinamico vacio");
        }
        return datos[tamanoActual - 1];
    }

    T& operator[](int indice) {
        return obtener(indice);
    }

    const T& operator[](int indice) const {
        return obtener(indice);
    }

    // Búsqueda genérica mediante condición (lambda o functor) sin usar STL
    template <typename Predicado>
    int buscarSi(Predicado condicion) const {
        for (int i = 0; i < tamanoActual; ++i) {
            if (condicion(datos[i])) {
                return i;
            }
        }
        return -1;
    }

    void limpiar() {
        tamanoActual = 0;
        if (capacidad > CAPACIDAD_MINIMA) {
            redimensionar(CAPACIDAD_MINIMA);
        }
    }

    int tamano() const {
        return tamanoActual;
    }

    int obtenerCapacidad() const {
        return capacidad;
    }

    bool estaVacio() const {
        return tamanoActual == 0;
    }
};

#endif