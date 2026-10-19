#pragma once
#include <iostream>
#include <string>
#include "Cliente.h"      // Es necesario porque se usa Cliente y NodoCola, definidos en cliente.h


// Cola con prioridad para la fila de espera VIP
// Se atiende primero al cliente con la tarjeta de mayor nivel
// Si dos clientes tienen la misma prioridad, se mantiene el orden de llegada (FIFO)


class ColaPrioridad{
  public:

    ColaPrioridad(){             // Constructor: crea una fila vacía
      cabeza = NULL;             // No hay primer nodo
      ultimo = NULL;             // No hay último nodo
    };

    ~ColaPrioridad(){                     // Destructor: libera la memoria de todos los nodos
      NodoCola * aux = cabeza;            // aux apunta al primer nodo
      while(!esvacia()){                  // Mientras queden nodos
        cabeza = cabeza->sig;             // La cabeza avanza al siguiente nodo
        delete aux;                       // Se libera el nodo anterior
        aux = cabeza;                     // aux se ubica en la nueva cabeza
      }
    };

    void encolar( Cliente c ){                               // Inserta al cliente justo después del último que tenga una prioridad mayor o igual a la suya (así los iguales respetan el orden de llegada)
      NodoCola * nuevo = new NodoCola(c);                    // Nodo con el cliente que llega

      if(esvacia()){                                         // Caso 1 → fila vacía
        cabeza = nuevo;                                      // El nuevo es el primero
        ultimo = nuevo;                                      // Y también el último
        return;                                              // Termina la función
      };

      if(c.prioridad() > cabeza->dato.prioridad()){          // Caso 2 → tiene más prioridad que todos
        nuevo->sig = cabeza;                                 // El nuevo apunta al que era primero
        cabeza = nuevo;                                      // El nuevo pasa a ser el primero
        return;
      };

                                                                                // Caso 3 →va en el medio o al final
      NodoCola * aux = cabeza;                                                  // Se busca la posición (empieza en el primer nodo)
      while(aux->sig!=NULL && aux->sig->dato.prioridad() >= c.prioridad()){     // Mientras el siguiente tenga prioridad mayor o igual
        aux = aux->sig;                                                         // Avanza un nodo
      };                                                                        // Al salir, el nuevo va justo después de aux
      nuevo->sig = aux->sig;                                                    // El nuevo apunta a lo que venía después de aux
      aux->sig = nuevo;                                                         // aux ahora apunta al nuevo
      if(aux==ultimo){ ultimo = nuevo; }                                        // Si se insertó al final, actualiza "ultimo"
    };

    bool desencolar( Cliente & c ){       // Se atiende al cliente de mayor prioridad (el primero de la fila)
      if(esvacia()) return false;         // Fila vacía → no hay a quién atender
      NodoCola * aux = cabeza;            // Guarda el primer nodo para borrarlo después
      c = cabeza->dato;                   // Entrega el cliente por la referencia
      cabeza = cabeza->sig;               // El siguiente pasa a ser el primero
      if(esvacia()){                      // Si la fila quedó vacía se limpia también "ultimo"
        ultimo = NULL;
      };
      delete aux;                         // Libera el nodo que salió
      return true;
    };

    bool frente( Cliente & c ){           // Consulta al primer cliente sin sacarlo de la fila
      if(esvacia()) return false;         // Fila vacía → no hay a quién consultar
      c = cabeza->dato;                   // Copia el primer cliente
      return true;
    };

    bool esvacia(){                       // Dice si la fila está vacía o no
      return cabeza==NULL;                // Vacía → no existe primer nodo
    };

    int longitud(){                       // Cuántos clientes hay en la fila
      int c = 0;                          // Contador
      NodoCola * aux = cabeza;            // Empieza en el primer nodo
      while(aux!=NULL){                   // Recorre hasta pasar el último nodo
        c++;                              // Cuenta un cliente
        aux = aux->sig;                   // Avanza al siguiente nodo
      };
      return c;
    };

    void mostrar(){                                                             // Imprime la fila (del primero al último)
      NodoCola * aux = cabeza;                                                  // Empieza en el primer nodo
      int k = 1;                                                                // Número de posición que se muestra
      while(aux!=NULL){                                                         // Recorre toda la fila
        std::cout << "  " << k << ". " << aux->dato.convertir() << '\n';        // Imprime "posición. datos"
        aux = aux->sig;                                                         // Avanza al siguiente nodo
        k++;                                                                    // Siguiente posición
      };
    };

  protected:
    NodoCola * cabeza;                    // Primer nodo → el de mayor prioridad
    NodoCola * ultimo;                    // Último nodo → el de menor prioridad
};
