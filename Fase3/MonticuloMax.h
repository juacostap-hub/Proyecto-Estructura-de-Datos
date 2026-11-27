#ifndef MONTICULOMAX_H    // Evita que este archivo se lea más de una vez al compilar
#define MONTICULOMAX_H    // Marca que este archivo ya fue leído


// MONTÍCULO MÁXIMO (Max Heap) hecho a mano, sobre un arreglo dinámico
// Es un árbol binario guardado dentro de un arreglo, con una regla:
//   "cada nodo tiene una clave MAYOR O IGUAL que la de sus dos hijos"
// Por eso la RAÍZ (posición 0) siempre es el elemento de mayor clave.
//
// Posiciones en el arreglo (se empieza en 0):
//   padre de i       →  (i - 1) / 2
//   hijo izquierdo   →  2 * i + 1
//   hijo derecho     →  2 * i + 2
//
// Cada elemento guarda dos números: un "dato" (qué es) y una "clave" (qué tan prioritario es).
// Se usa en dos lugares del juego:
//   1. Pedidos de la cocina: dato = número del pedido,  clave = -tiempoLimite
//   2. Dijkstra:             dato = número del nodo,    clave = -distancia
// Como el montículo es MÁXIMO y se quiere sacar lo MÁS PEQUEÑO (el tiempo límite más cercano,
// la distancia más corta), la clave se guarda con signo negativo: el mayor de los negativos es el menor real.
//
// Igual que los arreglos de la fase 1: empieza con espacio para 10, crece ×1,618 al llenarse
// y se achica cuando sobra mucho espacio.


class ElementoMonticulo{    // Lo que se guarda en cada posición del montículo
  public:

    ElementoMonticulo(){                          // Constructor vacío: crea un elemento en blanco
      dato = 0;
      clave = 0;
    };

    ElementoMonticulo(int _dato, int _clave){     // Constructor con datos: crea un elemento completo
      dato = _dato;                               // Copia el dato (número del pedido o del nodo)
      clave = _clave;                             // Copia la clave (el negativo del tiempo límite o de la distancia)
    };

  public:
    int dato;                         // Qué elemento es (número del pedido o del nodo)
    int clave;                        // Prioridad: la mayor clave sale primero
};


class MonticuloMax{
  public:

    MonticuloMax(){                   // Constructor: crea el montículo vacío
      N = 0;                          // Todavía no hay elementos
      tamano = 10;                    // Empieza con espacio para 10 elementos
      datos = new ElementoMonticulo[tamano];      // Pide la memoria con "new"
    };

    ~MonticuloMax(){                  // Destructor: se ejecuta cuando el montículo deja de existir
      delete[] datos;                 // Libera la memoria (así no queda memoria perdida)
    };

    MonticuloMax(const MonticuloMax &) = delete;                // Se prohíbe copiar el montículo (una copia compartiría el mismo arreglo y daría errores)
    MonticuloMax & operator=(const MonticuloMax &) = delete;    // Tampoco se permite asignar un montículo a otro

    void insertar( ElementoMonticulo e ){               // Agrega un elemento al montículo → O(log n)
      if(N == tamano){                                  // Si el arreglo está lleno
        int ntamano = (int) (tamano*aureo);             // Calcula el nuevo tamaño (1.618 veces el actual)
        ElementoMonticulo* ndatos = new ElementoMonticulo[ntamano];     // Crea el arreglo nuevo, más grande
        for(int i=0; i<tamano; i++){                    // Recorre los elementos que ya había
          ndatos[i] = datos[i];                         // Los copia al arreglo nuevo
        };
        delete[] datos;                                 // Borra el arreglo viejo
        datos = ndatos;                                 // Ahora "datos" apunta al arreglo nuevo
        tamano = ntamano;                               // Se guarda el nuevo tamaño
      };
      datos[N] = e;                                     // El elemento entra en la primera posición libre (la última hoja)
      N++;                                              // Ahora hay un elemento más
      subir(N-1);                                       // Se sube hasta que su padre tenga una clave mayor
    };

    bool extraer( ElementoMonticulo & salida ){         // Saca la raíz (la mayor clave) y la entrega en "salida" → O(log n)
      if(esvacia()) return false;                       // Retorna false si el montículo está vacío
      salida = datos[0];                                // Entrega la raíz por la referencia
      N--;                                              // Ahora hay un elemento menos
      datos[0] = datos[N];                              // El último elemento ocupa el lugar de la raíz
      bajar(0);                                         // Se baja hasta que sus hijos tengan una clave menor
      if(N*aureo*aureo<tamano){                         // Si sobra mucho espacio, se achica el arreglo
        int ntamano = (int) (N*aureo);                  // Calcula el nuevo tamaño
        if(ntamano<10) ntamano = 10;                    // Nunca menos de 10
        ElementoMonticulo* ndatos = new ElementoMonticulo[ntamano];     // Crea el arreglo nuevo, más pequeño
        for(int i=0; i<N; i++){                         // Recorre los elementos guardados
          ndatos[i] = datos[i];                         // Los copia al arreglo nuevo
        };
        delete[] datos;                                 // Borra el arreglo viejo
        datos = ndatos;                                 // Ahora "datos" apunta al arreglo nuevo
        tamano = ntamano;                               // Se guarda el nuevo tamaño
      };
      return true;                                      // Se sacó un elemento con éxito
    };

    bool raiz( ElementoMonticulo & salida ){            // Consulta la raíz sin sacarla → O(1)
      if(esvacia()) return false;                       // Montículo vacío → no hay nada que consultar
      salida = datos[0];                                // Copia la raíz
      return true;
    };

    bool esvacia(){                                     // Dice si el montículo está vacío o no
      return N == 0;
    };

    int longitud(){                                     // Cuántos elementos hay guardados
      return N;
    };

    bool obtenerEn( int i, ElementoMonticulo & salida ){  // AGREGADO para la interfaz gráfica: copia el elemento de la posición i del arreglo (sin sacarlo)
      if(i < 0 || i >= N) return false;                 // Posición fuera del montículo
      salida = datos[i];                                // Copia el elemento
      return true;
    };

    void vaciar(){                                      // Borra todos los elementos (por ejemplo, al empezar una jornada nueva)
      N = 0;                                            // Basta con olvidar los elementos: el arreglo se reutiliza
    };

  protected:

    void subir( int i ){                                // Sube el elemento de la posición i mientras sea mayor que su padre
      while(i > 0){                                     // Mientras no sea la raíz
        int padre = (i-1)/2;                            // Posición del padre
        if(datos[i].clave > datos[padre].clave){        // Si el hijo es mayor que el padre, rompe la regla del montículo
          ElementoMonticulo aux = datos[i];             // Se intercambian
          datos[i] = datos[padre];
          datos[padre] = aux;
          i = padre;                                    // Ahora se revisa desde la posición del padre
        }else{
          break;                                        // El padre ya es mayor → el montículo quedó bien
        };
      };
    };

    void bajar( int i ){                                // Baja el elemento de la posición i mientras sea menor que alguno de sus hijos
      while(true){
        int izq = 2*i + 1;                              // Hijo izquierdo
        int der = 2*i + 2;                              // Hijo derecho
        int mayor = i;                                  // Se busca el mayor entre el nodo y sus dos hijos
        if(izq < N && datos[izq].clave > datos[mayor].clave) mayor = izq;     // Solo se revisa el hijo si existe (posición < N)
        if(der < N && datos[der].clave > datos[mayor].clave) mayor = der;
        if(mayor == i) break;                           // El nodo ya es mayor que sus hijos → el montículo quedó bien
        ElementoMonticulo aux = datos[i];               // Se intercambia con el hijo mayor
        datos[i] = datos[mayor];
        datos[mayor] = aux;
        i = mayor;                                      // Ahora se revisa desde la posición del hijo
      };
    };

    ElementoMonticulo* datos;         // Arreglo donde se guarda el montículo
    int N;                            // Cuántos elementos hay guardados
    int tamano;                       // Cuántos elementos caben en este momento
    double aureo = 1.6180339;         // Razón áurea para crecer o achicar (igual que en la fase 1)
};

#endif                    // Fin de la protección contra lecturas repetidas
