#ifndef PROYECTO1ED_H     // Evita que este archivo se lea más de una vez al compilar
#define PROYECTO1ED_H     // Marca que este archivo ya fue leído

#include <iostream>       // Para escribir en pantalla (std::cout)
#include <string>         // Para usar texto (std::string)
using namespace std;      // Permite escribir string en lugar de std::string


// ESTRUCTURAS DE LA PERSONA 1
// Arreglos dinámicos: menú de platos y mesas.  Pila: comanda de cada mesa.
// "Dinámico" significa que el arreglo crece o se encoge solo cuando hace falta.
// Cuando se llena, se crea uno nuevo 1.618 veces más grande (razón áurea), se copian
// los datos y se borra el viejo. Cuando sobra mucho espacio, se hace al revés.


// CLASES DE DATOS (lo que se guarda dentro de las estructuras)

class Plato{                          // Un plato del menú
  public:
    int id;                           // Número único del plato
    string nombre;                    // Nombre del plato
    double precio;                    // Precio del plato

    Plato() : id(0), nombre(""), precio(0.0) {}            // Constructor vacío: crea un plato en blanco
    Plato(int _id, string _nombre, double _precio) {       // Constructor con datos: crea un plato completo
        id = _id; nombre = _nombre; precio = _precio;      // Copia los datos recibidos
    }
};

class Mesa{                           // Una mesa del restaurante
  public:
    int id;                           // Número de la mesa
    int capacidad;                    // Cuántas personas caben
    bool ocupada;                     // true = hay clientes sentados, false = libre

    Mesa() : id(0), capacidad(0), ocupada(false) {}        // Constructor vacío: mesa en blanco
    Mesa(int _id, int _capacidad) {                        // Constructor con datos
        id = _id; capacidad = _capacidad; ocupada = false; // Copia los datos; toda mesa nueva empieza libre
    }
};

class Pedidos{                        // Un producto anotado en la comanda de una mesa
  public:
    Plato platoPedido;                // El plato que se pidió
    int numeroMesa;                   // Número de la mesa que lo pidió

    Pedidos() : platoPedido(), numeroMesa(0) {}            // Constructor vacío
    Pedidos(Plato _platoPedido, int _numeroMesa) {         // Constructor con datos
        platoPedido = _platoPedido; numeroMesa = _numeroMesa;      // Copia el plato y la mesa
    }
};


// ARREGLO DINÁMICO DEL MENÚ

class ArregloMenu{
  public:
    ArregloMenu(){                    // Constructor: crea el arreglo vacío
      N = 0;                          // Todavía no hay platos
      tamano = 10;                    // Empieza con espacio para 10 platos
      datos = new Plato[tamano];      // Pide la memoria con "new"
    }

    ~ArregloMenu(){                   // Destructor: se ejecuta cuando el arreglo deja de existir
      delete[] datos;                 // Libera la memoria (así no queda memoria perdida)
    }

    void agregar(Plato p){                              // Agrega un plato al final del menú
      if(N == tamano){                                  // Si el arreglo está lleno
        int ntamano = (int) (tamano*aureo);             // Calcula el nuevo tamaño (1.618 veces el actual)
        Plato* ndatos = new Plato[ntamano];             // Crea el arreglo nuevo, más grande
        for(int i=0; i<tamano; i++){                    // Recorre los platos que ya había
          ndatos[i] = datos[i];                         // Los copia al arreglo nuevo
        }
        delete[] datos;                                 // Borra el arreglo viejo
        datos = ndatos;                                 // Ahora "datos" apunta al arreglo nuevo
        tamano = ntamano;                               // Se guarda el nuevo tamaño
      }
      datos[N] = p;                                     // Guarda el plato en la primera posición libre
      N++;                                              // Ahora hay un plato más
    }

    void borrar(){                                      // Quita el último plato del menú
      if(N > 0){                                        // CORRECCIÓN: si el menú está vacío no hay nada que borrar (antes N podía quedar en -1)
        if(N*aureo*aureo<tamano){                       // Si sobra mucho espacio, se achica el arreglo
          int ntamano = (int) (N*aureo);                // Calcula el nuevo tamaño
          if(ntamano<10) ntamano = 10;                  // CORRECCIÓN: nunca menos de 10 (con tamaño 1 el arreglo ya no podía crecer y se escribía fuera de la memoria)
          Plato* ndatos = new Plato[ntamano];           // Crea el arreglo nuevo, más pequeño
          for(int i=0; i<N; i++){                       // Recorre los platos guardados
            ndatos[i] = datos[i];                       // Los copia al arreglo nuevo
          }
          delete[] datos;                               // Borra el arreglo viejo
          datos = ndatos;                               // Ahora "datos" apunta al arreglo nuevo
          tamano = ntamano;                             // Se guarda el nuevo tamaño
        }
        N--;                                            // Ahora hay un plato menos (el último queda "olvidado")
      }
    }

    int longitud(){                                     // Cuántos platos hay en el menú
      return N;
    }

    Plato obtener(int i){                               // Devuelve el plato de la posición i
      return datos[i];
    }

    int buscar(string nombreBuscar){                    // Busca un plato por su nombre
      for(int i=0; i<N; i++){                           // Recorre todos los platos
        if(datos[i].nombre == nombreBuscar){            // Si el nombre coincide
          return i;                                     // Devuelve la posición donde está
        }
      }
      return -1;                                        // -1 significa que no se encontró
    }

  protected:
    Plato* datos;                     // Arreglo donde se guardan los platos
    int N;                            // Cuántos platos hay guardados
    int tamano;                       // Cuántos platos caben en este momento
    double aureo = 1.6180339;         // Razón áurea: factor para crecer o achicar el arreglo
};


// ARREGLO DINÁMICO DE MESAS

class ArregloMesa{
  public:
    ArregloMesa(){                    // Constructor: crea el arreglo vacío
      N = 0;                          // Todavía no hay mesas
      tamano = 10;                    // Empieza con espacio para 10 mesas
      datos = new Mesa[tamano];       // Pide la memoria con "new"
    }

    ~ArregloMesa(){                   // Destructor
      delete[] datos;                 // Libera la memoria
    }

    void agregar(Mesa m){                               // Agrega una mesa al final
      if(N == tamano){                                  // Si el arreglo está lleno
        int ntamano = (int) (tamano*aureo);             // Calcula el nuevo tamaño
        Mesa* ndatos = new Mesa[ntamano];               // Crea el arreglo nuevo, más grande
        for(int i=0; i<tamano; i++){                    // Recorre las mesas que ya había
          ndatos[i] = datos[i];                         // Las copia al arreglo nuevo
        }
        delete[] datos;                                 // Borra el arreglo viejo
        datos = ndatos;                                 // Ahora "datos" apunta al arreglo nuevo
        tamano = ntamano;                               // Se guarda el nuevo tamaño
      }
      datos[N] = m;                                     // Guarda la mesa en la primera posición libre
      N++;                                              // Ahora hay una mesa más
    }

    void borrar(){                                      // Quita la última mesa
      if(N > 0){                                        // CORRECCIÓN: si no hay mesas no hay nada que borrar
        if(N*aureo*aureo<tamano){                       // Si sobra mucho espacio, se achica el arreglo
          int ntamano = (int) (N*aureo);                // Calcula el nuevo tamaño
          if(ntamano<10) ntamano = 10;                  // CORRECCIÓN: nunca menos de 10
          Mesa* ndatos = new Mesa[ntamano];             // Crea el arreglo nuevo, más pequeño
          for(int i=0; i<N; i++){                       // Recorre las mesas guardadas
            ndatos[i] = datos[i];                       // Las copia al arreglo nuevo
          }
          delete[] datos;                               // Borra el arreglo viejo
          datos = ndatos;                               // Ahora "datos" apunta al arreglo nuevo
          tamano = ntamano;                             // Se guarda el nuevo tamaño
        }
        N--;                                            // Ahora hay una mesa menos
      }
    }

    int longitud(){                                     // Cuántas mesas hay
      return N;
    }

    Mesa obtener(int i){                                // Devuelve una COPIA de la mesa en la posición i
      return datos[i];
    }

    int buscar(int idBuscar){                           // Busca una mesa por su número
      for(int i=0; i<N; i++){                           // Recorre todas las mesas
        if(datos[i].id == idBuscar){                    // Si el número coincide
          return i;                                     // Devuelve la posición donde está
        }
      }
      return -1;                                        // -1 significa que no se encontró
    }

    void cambiarEstado(int i, bool ocupada){            // AGREGADO en la integración: marca la mesa i como ocupada o libre
      if(i >= 0 && i < N){                              // Solo si la posición existe
        datos[i].ocupada = ocupada;                     // Cambia el estado dentro del arreglo (obtener() solo entrega una copia)
      }
    }

  protected:
    Mesa* datos;                      // Arreglo donde se guardan las mesas
    int N;                            // Cuántas mesas hay guardadas
    int tamano;                       // Cuántas mesas caben en este momento
    double aureo = 1.6180339;         // Razón áurea para crecer o achicar
};


// PILA DE PEDIDOS (COMANDA)
// Pila = el último en entrar es el primero en salir (LIFO).
// Está hecha sobre un arreglo dinámico: el "tope" de la pila es la última posición ocupada.

class PilaPedidos{
  public:
    PilaPedidos(){                    // Constructor: crea la pila vacía
      N = 0;                          // Todavía no hay pedidos
      tamano = 10;                    // Empieza con espacio para 10 pedidos
      datos = new Pedidos[tamano];    // Pide la memoria con "new"
    }

    ~PilaPedidos(){                   // Destructor
      delete[] datos;                 // Libera la memoria
    }

    void agregar(Pedidos p){                            // Apilar (push): pone un pedido encima de la pila
      if(N==tamano){                                    // Si el arreglo está lleno
        int ntamano = (int) (tamano*aureo);             // Calcula el nuevo tamaño
        Pedidos* ndatos = new Pedidos[ntamano];         // Crea el arreglo nuevo, más grande
        for(int i=0; i<tamano; i++){                    // Recorre los pedidos que ya había
          ndatos[i] = datos[i];                         // Los copia al arreglo nuevo
        }
        delete[] datos;                                 // Borra el arreglo viejo
        datos = ndatos;                                 // Ahora "datos" apunta al arreglo nuevo
        tamano = ntamano;                               // Se guarda el nuevo tamaño
      }
      datos[N] = p;                                     // El pedido queda en el tope de la pila
      N++;                                              // Ahora hay un pedido más
    }

    void borrar(){                                      // Desapilar (pop): quita el pedido del tope
      if(N>0){                                          // Solo si la pila no está vacía
        if(N*aureo*aureo<tamano){                       // Si sobra mucho espacio, se achica el arreglo
          int ntamano = (int) (N*aureo);                // Calcula el nuevo tamaño
          if(ntamano<10) ntamano = 10;                  // Nunca menos de 10
          Pedidos* ndatos = new Pedidos[ntamano];       // Crea el arreglo nuevo, más pequeño
          for(int i=0; i<N; i++){                       // Recorre los pedidos guardados
            ndatos[i] = datos[i];                       // Los copia al arreglo nuevo
          }
          delete[] datos;                               // Borra el arreglo viejo
          datos = ndatos;                               // Ahora "datos" apunta al arreglo nuevo
          tamano = ntamano;                             // Se guarda el nuevo tamaño
        }
        N--;                                            // Ahora hay un pedido menos (el del tope)
      }
    }

    Pedidos& top() {                                    // Consulta el pedido del tope SIN quitarlo (solo usar si la pila no está vacía)
        return datos[N - 1];                            // El tope es la última posición ocupada
    }

    bool estaVacia() {                                  // Dice si no hay ningún pedido
        return N == 0;
    }

    int cantidad() {                                    // Cuántos pedidos hay en la pila
        return N;
    }

  protected:
    Pedidos* datos;                   // Arreglo donde se guardan los pedidos
    int N;                            // Cuántos pedidos hay guardados
    int tamano;                       // Cuántos pedidos caben en este momento
    double aureo = 1.6180339;         // Razón áurea para crecer o achicar
};

#endif                    // Fin de la protección contra lecturas repetidas
