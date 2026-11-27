
// MATRIZ DISPERSA

#ifndef MATRIZ_DISPERSA_H
#define MATRIZ_DISPERSA_H

#include <cstddef>
#include <iostream>
#include "ArregloDinamico.h"

using namespace std;

enum TipoCasilla {
    VACIO = 0,
    PARED,
    MESA,
    COCINA,
    CAJA,
    PUERTA,
    OBSTACULO
};

struct NodoCasilla {
    int fila;
    int col;
    TipoCasilla tipo;
    int idObjeto;        // Ej: numero de mesa (0, 1, 2...), -1 si no aplica
    int ticksRestantes;  // -1 para objetos permanentes; > 0 para obstaculos temporales
    NodoCasilla* siguiente;

    NodoCasilla(int f, int c, TipoCasilla t, int id = -1, int ticks = -1)
        : fila(f), col(c), tipo(t), idObjeto(id), ticksRestantes(ticks), siguiente(nullptr) {}
};

class MatrizDispersa {
private:
    ArregloDinamico<NodoCasilla*> filas;
    int numFilas;
    int numCols;
    int totalNodos;

    bool enRango(int fila, int col) const {
        return fila >= 0 && fila < numFilas && col >= 0 && col < numCols;
    }

    void copiarDesde(const MatrizDispersa& otra) {
        numFilas = otra.numFilas;
        numCols = otra.numCols;
        totalNodos = 0;

        for (int f = 0; f < numFilas; ++f) {
            filas.agregar(nullptr);
            NodoCasilla* actualOtra = otra.filas[f];
            NodoCasilla* ultimoNuevo = nullptr;

            while (actualOtra != nullptr) {
                NodoCasilla* copia = new NodoCasilla(
                    actualOtra->fila,
                    actualOtra->col,
                    actualOtra->tipo,
                    actualOtra->idObjeto,
                    actualOtra->ticksRestantes
                );

                if (filas[f] == nullptr) {
                    filas[f] = copia;
                } else {
                    ultimoNuevo->siguiente = copia;
                }
                ultimoNuevo = copia;
                ++totalNodos;
                actualOtra = actualOtra->siguiente;
            }
        }
    }

public:
    // Por defecto: mapa de 10 filas x 15 columnas (15x10)
    MatrizDispersa(int f = 10, int c = 15) : numFilas(f), numCols(c), totalNodos(0) {
        if (numFilas < 1) numFilas = 10;
        if (numCols < 1) numCols = 15;

        for (int i = 0; i < numFilas; ++i) {
            filas.agregar(nullptr);
        }
    }

    // Regla de Tres: necesaria porque ArregloDinamico solo guarda punteros
    MatrizDispersa(const MatrizDispersa& otra) : numFilas(0), numCols(0), totalNodos(0) {
        copiarDesde(otra);
    }

    MatrizDispersa& operator=(const MatrizDispersa& otra) {
        if (this == &otra) {
            return *this;
        }
        limpiarTodo();
        filas.limpiar();
        copiarDesde(otra);
        return *this;
    }

    ~MatrizDispersa() {
        limpiarTodo();
    }

    // Libera todos los nodos dinamicos conservando las filas vacias
    void limpiarTodo() {
        for (int f = 0; f < filas.tamano(); ++f) {
            NodoCasilla* actual = filas[f];
            while (actual != nullptr) {
                NodoCasilla* aBorrar = actual;
                actual = actual->siguiente;
                delete aBorrar;
            }
            filas[f] = nullptr;
        }
        totalNodos = 0;
    }

    // Inserta manteniendo la lista de la fila ordenada por columna.
    // Retorna false si esta fuera de rango o la casilla ya esta ocupada.
    bool insertar(int fila, int col, TipoCasilla tipo, int idObjeto = -1, int ticks = -1) {
        if (!enRango(fila, col) || tipo == VACIO) {
            return false;
        }

        NodoCasilla* anterior = nullptr;
        NodoCasilla* actual = filas[fila];

        while (actual != nullptr && actual->col < col) {
            anterior = actual;
            actual = actual->siguiente;
        }

        // Evita duplicados en la misma coordenada
        if (actual != nullptr && actual->col == col) {
            return false;
        }

        NodoCasilla* nuevo = new NodoCasilla(fila, col, tipo, idObjeto, ticks);
        nuevo->siguiente = actual;

        if (anterior == nullptr) {
            filas[fila] = nuevo;
        } else {
            anterior->siguiente = nuevo;
        }

        ++totalNodos;
        return true;
    }

    // Elimina una casilla ocupada (ej. al expirar un obstaculo)
    bool eliminar(int fila, int col) {
        if (!enRango(fila, col)) {
            return false;
        }

        NodoCasilla* anterior = nullptr;
        NodoCasilla* actual = filas[fila];

        while (actual != nullptr && actual->col < col) {
            anterior = actual;
            actual = actual->siguiente;
        }

        if (actual == nullptr || actual->col != col) {
            return false;
        }

        if (anterior == nullptr) {
            filas[fila] = actual->siguiente;
        } else {
            anterior->siguiente = actual->siguiente;
        }

        delete actual;
        --totalNodos;
        return true;
    }

    // Busca una casilla en O(K_f), deteniendose si pasa la columna
    NodoCasilla* consultar(int fila, int col) const {
        if (!enRango(fila, col)) {
            return nullptr;
        }

        NodoCasilla* actual = filas[fila];
        while (actual != nullptr && actual->col <= col) {
            if (actual->col == col) {
                return actual;
            }
            actual = actual->siguiente;
        }
        return nullptr;
    }

    TipoCasilla obtenerTipo(int fila, int col) const {
        NodoCasilla* nodo = consultar(fila, col);
        return (nodo != nullptr) ? nodo->tipo : VACIO;
    }

    // Indica si el jugador puede caminar hacia (fila, col)
    bool esTransitable(int fila, int col) const {
        if (!enRango(fila, col)) {
            return false;
        }
        return consultar(fila, col) == nullptr;
    }

    // Valida la regla del ataque "Caos en el restaurante":
    // no ocupar casillas ya usadas ni bloquear el acceso adyacente a puerta, cocina o caja
    bool esPosicionSeguraParaObstaculo(int fila, int col, int jugadorFila = -1, int jugadorCol = -1) const {
        if (!esTransitable(fila, col)) {
            return false;
        }
        if (fila == jugadorFila && col == jugadorCol) {
            return false;
        }

        const int df[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};

        for (int i = 0; i < 4; ++i) {
            TipoCasilla vecino = obtenerTipo(fila + df[i], col + dc[i]);
            if (vecino == PUERTA || vecino == COCINA || vecino == CAJA) {
                return false;
            }
        }
        return true;
    }

    // Se llama en avanzarTick(): descuenta vida a los obstaculos y borra los que llegan a 0.
    // Retorna cuantos obstaculos desaparecieron en este tick.
    int actualizarObstaculos() {
        int eliminados = 0;

        for (int f = 0; f < numFilas; ++f) {
            NodoCasilla* anterior = nullptr;
            NodoCasilla* actual = filas[f];

            while (actual != nullptr) {
                if (actual->tipo == OBSTACULO && actual->ticksRestantes > 0) {
                    actual->ticksRestantes--;

                    if (actual->ticksRestantes == 0) {
                        NodoCasilla* aBorrar = actual;
                        if (anterior == nullptr) {
                            filas[f] = actual->siguiente;
                        } else {
                            anterior->siguiente = actual->siguiente;
                        }
                        actual = actual->siguiente;
                        delete aBorrar;
                        --totalNodos;
                        ++eliminados;
                        continue;
                    }
                }
                anterior = actual;
                actual = actual->siguiente;
            }
        }

        return eliminados;
    }

    // Acceso a la cabecera de una fila para serializar facil a JSON en el servidor local
    const NodoCasilla* obtenerCabezaFila(int fila) const {
        if (fila < 0 || fila >= numFilas) {
            return nullptr;
        }
        return filas[fila];
    }

    int obtenerFilas() const { return numFilas; }
    int obtenerColumnas() const { return numCols; }
    int obtenerTotalNodos() const { return totalNodos; }

    // Metricas para la comparacion de memoria en el informe y pantalla de estadisticas
    size_t memoriaDispersaBytes() const {
        size_t memCabeceras = static_cast<size_t>(filas.obtenerCapacidad()) * sizeof(NodoCasilla*);
        size_t memNodos = static_cast<size_t>(totalNodos) * sizeof(NodoCasilla);
        return sizeof(MatrizDispersa) + memCabeceras + memNodos;
    }

    size_t memoriaDensaEquivalenteBytes() const {
        struct CeldaDensa {
            TipoCasilla tipo;
            int idObjeto;
            int ticksRestantes;
        };
        return static_cast<size_t>(numFilas) * static_cast<size_t>(numCols) * sizeof(CeldaDensa);
    }

    // Utilidad de depuracion rapida en consola
    void imprimirConsola(int jugadorFila = -1, int jugadorCol = -1) const {
        for (int f = 0; f < numFilas; ++f) {
            NodoCasilla* actual = filas[f];
            for (int c = 0; c < numCols; ++c) {
                while (actual != nullptr && actual->col < c) {
                    actual = actual->siguiente;
                }

                if (f == jugadorFila && c == jugadorCol) {
                    cout << "@ ";
                } else if (actual != nullptr && actual->col == c) {
                    switch (actual->tipo) {
                        case PARED:     cout << "# "; break;
                        case MESA:      cout << "M "; break;
                        case COCINA:    cout << "K "; break;
                        case CAJA:      cout << "$ "; break;
                        case PUERTA:    cout << "P "; break;
                        case OBSTACULO: cout << "X "; break;
                        default:        cout << ". "; break;
                    }
                } else {
                    cout << ". ";
                }
            }
            cout << "\n";
        }
    }
};

#endif