#pragma once
#include <iostream>
#include <string>
#include "Cliente.h"      // Es necesario porque se usa Cliente y NodoCola, definidos en cliente.h


// Cola normal (FIFO - First In First Out)
// El primer cliente que llega es el primero en atenderse


class Cola{
  public:

    Cola(){                               // Constructor vacío: Crea una cola vacía
      cabeza = NULL;                      // No hay primer nodo
      ultimo = NULL;                      // No hay último nodo
    };

    ~Cola(){                              // Destructor: Libera la memoria ocupada por los nodos
      NodoCola * aux = cabeza;            // aux apunta al primer nodo
      while(!esvacia()){                  // Mientras queden nodos
        cabeza = cabeza->sig;             // La cabeza avanza al siguiente nodo
        delete aux;                       // Se libera el nodo anterior
        aux = cabeza;                     // aux se ubica en la nueva cabeza
      }
    };

    void encolar( Cliente c ){                  // Agrega un cliente al final de la fila
      NodoCola * aux = new NodoCola(c);         // Crea un nodo nuevo en la memoria dinámica (con "new")
      if(esvacia()){                            // Caso 1 → la fila estaba vacía
        cabeza = aux;                           // El nodo nuevo es también el primero
      }else{                                    // Caso 2 → ya había clientes
        ultimo->sig = aux;                      // El último actual apunta al nodo nuevo
      };
      ultimo = aux;                             // En ambos casos el nodo nuevo pasa a ser el último
    };

    bool desencolar( Cliente & c ){         // Saca al primer cliente de la fila y lo devuelve en c
      if(esvacia()) return false;           // Retorna false si la fila está vacía
      NodoCola * aux = cabeza;              // Guarda el primer nodo para poder borrarlo después
      c = cabeza->dato;                     // Entrega el cliente por la referencia
      cabeza = cabeza->sig;                 // La cabeza avanza al siguiente nodo
      if(esvacia()){
        ultimo = NULL;                     // Si con esto la fila quedó vacía también "ultimo" debe quedar en NULL
      };
      delete aux;                          // Libera el nodo que ya salió
      return true;                         // Se atendió un cliente con éxito
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
    NodoCola * cabeza;                    // Primer nodo de la fila (el próximo en atenderse)
    NodoCola * ultimo;                    // Último nodo de la fila (donde se agrega al llegar)
};
