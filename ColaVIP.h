#pragma once
#include <iostream>
#include <string>
#include "Cliente.h"      // Es necesario porque se usa Cliente, definido en Cliente.h
#include "colaComun.h"    // Es necesario porque cada nivel de la fila VIP es una Cola, definida en colaComun.h
 
 
// Cola con prioridad para la fila de espera VIP
// Se usa un arreglo de colas: cada posición del arreglo es la cola de un nivel de tarjeta
// Se atiende primero al cliente del nivel más alto que tenga gente esperando
// Dentro de un mismo nivel se mantiene el orden de llegada (FIFO)
 
 
class ColaPrioridad{
  public:
 
    ColaPrioridad(){             // Constructor: crea una fila vacía
    };

    // Las colas del arreglo ya nacen vacías (su propio constructor las inicializa)
    // No hace falta destructor: cada Cola del arreglo libera sus propios nodos al destruirse
 
    void encolar( Cliente c ){                               // Inserta al cliente al final de la cola de su nivel de tarjeta
      int nivel = c.prioridad();                             // Nivel de la tarjeta: 1, 2 o 3 (el índice del arreglo)
      if(nivel < 0){ nivel = 0; };                           // Por seguridad: nunca se sale del arreglo por abajo
      if(nivel > PRESTIGIO){ nivel = PRESTIGIO; };           // Por seguridad: nunca se sale del arreglo por arriba
      niveles[nivel].encolar(c);                             // Al final de la cola de ese nivel (así los iguales respetan el orden de llegada)
    };
 
    bool desencolar( Cliente & c ){                          // Se atiende al cliente de mayor prioridad
      for(int i=PRESTIGIO; i>=0; i--){                       // Se revisa desde el nivel más alto hasta el más bajo
        if(!niveles[i].esvacia()){                           // El primer nivel que tenga gente...
          return niveles[i].desencolar(c);                   // ...entrega a su primer cliente por la referencia
        };
      };
      return false;                                          // Todos los niveles vacíos → no hay a quién atender
    };
 
    bool frente( Cliente & c ){                              // Consulta al próximo cliente sin sacarlo de la fila
      for(int i=PRESTIGIO; i>=0; i--){                       // Mismo recorrido de mayor a menor nivel
        if(!niveles[i].esvacia()){
          return niveles[i].frente(c);                       // Copia el primer cliente de ese nivel
        };
      };
      return false;                                          // Fila vacía → no hay a quién consultar
    };
 
    bool esvacia(){                                          // Dice si la fila está vacía o no
      for(int i=0; i<=PRESTIGIO; i++){                       // Se revisan todos los niveles
        if(!niveles[i].esvacia()) return false;              // Con que uno tenga gente, la fila no está vacía
      };
      return true;                                           // Todos los niveles están vacíos
    };
 
    int longitud(){                                          // Cuántos clientes hay en la fila
      int c = 0;                                             // Contador
      for(int i=0; i<=PRESTIGIO; i++){                       // Se recorren todos los niveles
        c = c + niveles[i].longitud();                       // Se suman los clientes de cada nivel
      };
      return c;
    };
 
    void mostrar(){                                                             // Imprime la fila en el orden en que se atenderá
      int k = 1;                                                                // Número de posición que se muestra
      for(int i=PRESTIGIO; i>=0; i--){                                          // De mayor a menor nivel
        int n = niveles[i].longitud();                                          // Cuántos clientes tiene este nivel
        for(int j=0; j<n; j++){                                                 // Se saca y se vuelve a meter cada uno: así la cola queda igual
          Cliente c;
          niveles[i].desencolar(c);                                             // Sale el primero del nivel
          std::cout << "  " << k << ". " << c.convertir() << '\n';              // Imprime "posición. datos"
          niveles[i].encolar(c);                                                // Vuelve al final (tras n vueltas queda en su orden original)
          k++;                                                                  // Siguiente posición
        };
      };
    };
 
  protected:
    Cola niveles[PRESTIGIO + 1];          // Arreglo de colas: posición 0 = sin tarjeta, 1 = frecuente, 2 = amigo del gerente, 3 = prestigio
};
