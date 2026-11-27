#ifndef GRAFO_H           // Evita que este archivo se lea más de una vez al compilar
#define GRAFO_H           // Marca que este archivo ya fue leído

#include <string>         // Para usar texto (std::string)
#include <cstdlib>        // Para rand() (sorteo de costos)
#include <cstddef>        // Para la constante NULL
#include "MonticuloMax.h" // Es necesario porque Dijkstra usa un montículo máximo


// GRAFO DE LA CIUDAD (turno de noche) + ALGORITMO DE DIJKSTRA
// La ciudad es un grafo fijo de 11 nodos, guardado con LISTA DE ADYACENCIA:
//   nodo 0        → el restaurante
//   nodos 1 a 4   → intersecciones
//   nodos 5 a 10  → casas (destinos de los domicilios)
// Las calles son aristas SIN dirección (se puede ir y volver) y cada una tiene un costo en ticks.
// La forma de la ciudad no cambia, pero los costos se sortean al empezar cada jornada (de 1 a 5),
// por eso la mejor ruta cambia de una jornada a otra sin reconstruir el mapa.
//
// Para que el costo de una calle sea UNO SOLO (aunque se pueda recorrer en los dos sentidos),
// cada calle tiene un número y su costo se guarda en el arreglo costo[]. Las dos aristas
// de esa calle (ida y vuelta) apuntan al mismo número, así que al cambiar el costo cambian las dos.
//
// De noche, Jonatan puede CERRAR una calle: queda marcada en cerrada[] y Dijkstra la salta.
// Antes de cerrarla se revisa con Dijkstra que todos los lugares sigan conectados (nunca deja una casa aislada).
// Las calles a las que el ataque Tráfico les subió el costo quedan marcadas en lenta[] (solo para dibujarlas).


const int MAX_NODOS   = 20;          // Cantidad máxima de nodos del grafo
const int MAX_CALLES  = 40;          // Cantidad máxima de calles
const int INFINITO    = 1000000;     // Distancia de un nodo al que todavía no se ha llegado
const int RESTAURANTE = 0;           // El nodo 0 siempre es el restaurante
const int COSTO_MIN   = 1;           // Costo mínimo de una calle (Dijkstra exige costos positivos)
const int COSTO_MAX   = 5;           // Costo máximo al sortear los costos de una jornada


class Arista{       // Nodo de la lista de adyacencia: una calle que sale de un nodo
  public:

    Arista(int _destino, int _calle){       // Crea una arista hacia "_destino" por la calle número "_calle"
      destino = _destino;                   // Nodo al que se llega
      calle = _calle;                       // Número de la calle (para buscar su costo)
      sig = NULL;                           // Al principio no apunta a ninguna otra arista
    };

  public:
    int destino;                            // Nodo al que lleva esta calle
    int calle;                              // Número de la calle
    Arista * sig;                           // Siguiente arista de la lista (NULL si es la última)
};


class Grafo{
  public:

    Grafo(){                                // Constructor: crea la ciudad fija de 11 nodos con costos iniciales
      numNodos = 0;
      numCalles = 0;
      origenCalculado = -1;                 // Todavía no se ha corrido Dijkstra
      for(int i=0; i<MAX_NODOS; i++){       // Todas las listas empiezan vacías
        ady[i] = NULL;
        dist[i] = INFINITO;
        previo[i] = -1;
      };
      for(int i=0; i<MAX_CALLES; i++){      // Ninguna calle cerrada ni lenta
        cerrada[i] = false;
        lenta[i] = false;
      };
      armarCiudad();                        // Crea los nodos y las calles
      sortearCostos();                      // Primeros costos (los de la jornada 1)
    };

    ~Grafo(){                               // Destructor: libera todas las aristas de las listas
      for(int i=0; i<numNodos; i++){        // Recorre cada nodo
        Arista * aux = ady[i];              // aux apunta a la primera arista del nodo
        while(aux != NULL){                 // Mientras queden aristas
          Arista * borrar = aux;            // Guarda la arista actual
          aux = aux->sig;                   // Avanza a la siguiente
          delete borrar;                    // Libera la anterior
        };
        ady[i] = NULL;
      };
    };

    Grafo(const Grafo &) = delete;                  // Se prohíbe copiar el grafo (una copia compartiría las listas y daría errores)
    Grafo & operator=(const Grafo &) = delete;      // Tampoco se permite asignar un grafo a otro


    // ---------- CONSTRUCCIÓN DEL MAPA ----------

    int agregarNodo(std::string nombre){            // Agrega un nodo y devuelve su número (-1 si ya no caben más)
      if(numNodos >= MAX_NODOS) return -1;
      nombres[numNodos] = nombre;                   // Guarda el nombre (para mostrarlo en pantalla)
      ady[numNodos] = NULL;                         // Lista de calles vacía
      numNodos++;
      return numNodos-1;
    };

    int agregarCalle(int a, int b, int _costo){     // Une los nodos a y b con una calle de ida y vuelta. Devuelve el número de la calle (-1 si no se pudo)
      if(a<0 || b<0 || a>=numNodos || b>=numNodos || a==b) return -1;      // Nodos que no existen
      if(numCalles >= MAX_CALLES) return -1;                                // Ya no caben más calles
      if(_costo < COSTO_MIN) _costo = COSTO_MIN;                            // Nunca costos menores a 1
      int id = numCalles;                           // Número de la calle nueva
      extremoA[id] = a;                             // Se guardan sus dos extremos
      extremoB[id] = b;
      costo[id] = _costo;                           // Y su costo
      numCalles++;
      Arista * ida = new Arista(b, id);             // Arista de a hacia b
      ida->sig = ady[a];                            // Se inserta al INICIO de la lista (así no hay que recorrerla)
      ady[a] = ida;
      Arista * vuelta = new Arista(a, id);          // Arista de b hacia a (misma calle)
      vuelta->sig = ady[b];
      ady[b] = vuelta;
      return id;
    };


    // ---------- COSTOS DE LAS CALLES ----------

    void sortearCostos(){                           // Sortea el costo de cada calle entre 1 y 5 (se llama al empezar cada noche)
      for(int i=0; i<numCalles; i++){
        costo[i] = COSTO_MIN + rand() % (COSTO_MAX - COSTO_MIN + 1);
        cerrada[i] = false;                         // Noche nueva: todas las calles abiertas...
        lenta[i] = false;                           // ...y sin tráfico
      };
      origenCalculado = -1;                         // Las distancias que había ya no sirven
    };

    void escalarCostos(int porcentaje){             // Multiplica todos los costos por porcentaje/100 (jornada 3: 150 = costos +50%). Se redondea hacia arriba
      for(int i=0; i<numCalles; i++){
        costo[i] = (costo[i] * porcentaje + 99) / 100;      // +99 hace que la división entera redondee hacia arriba
        if(costo[i] < COSTO_MIN) costo[i] = COSTO_MIN;
      };
      origenCalculado = -1;
    };

    void sumarCosto(int calle, int aumento){        // Sube (o baja) el costo de una calle. Nunca queda menor a 1
      if(calle<0 || calle>=numCalles) return;
      costo[calle] = costo[calle] + aumento;
      if(costo[calle] < COSTO_MIN) costo[calle] = COSTO_MIN;
      origenCalculado = -1;
    };

    void traficoInfernal(int cantidad, int aumento){        // Ataque de Jonatan (Tráfico): sube "aumento" puntos el costo de "cantidad" calles distintas escogidas al azar
      if(cantidad > numCalles) cantidad = numCalles;
      int orden[MAX_CALLES];                        // Los números de las calles, para escoger sin repetir
      for(int i=0; i<numCalles; i++) orden[i] = i;
      for(int i=0; i<cantidad; i++){                // Se escogen "cantidad" calles
        int j = i + rand() % (numCalles - i);       // Una calle al azar entre las que no se han escogido
        int aux = orden[i]; orden[i] = orden[j]; orden[j] = aux;        // Se pone en la posición i
        sumarCosto(orden[i], aumento);              // Se le sube el costo
        lenta[orden[i]] = true;                     // Se marca para dibujarla con tráfico
      };
    };


    // ---------- CALLES CERRADAS (noche) ----------

    void cerrarCalle(int calle){                    // Jonatan cierra una calle: Dijkstra ya no pasa por ella
      if(calle<0 || calle>=numCalles) return;
      cerrada[calle] = true;
      origenCalculado = -1;
    };

    void abrirTodas(){                              // Vuelve a abrir todas las calles
      for(int i=0; i<numCalles; i++) cerrada[i] = false;
      origenCalculado = -1;
    };

    bool estaCerrada(int calle){                    // ¿Está cerrada esa calle?
      if(calle<0 || calle>=numCalles) return true;
      return cerrada[calle];
    };

    bool esLenta(int calle){                        // ¿Le subió el costo el ataque Tráfico?
      if(calle<0 || calle>=numCalles) return false;
      return lenta[calle];
    };

    bool todoConectado(){                           // ¿Se puede llegar del restaurante a TODOS los lugares? (usa Dijkstra)
      calcularRutas(RESTAURANTE);
      for(int i=0; i<numNodos; i++){
        if(dist[i] >= INFINITO) return false;       // Un lugar quedó aislado
      };
      return true;
    };

    bool sePuedeCerrar(int calle){                  // ¿Se puede cerrar esa calle sin dejar ningún lugar aislado?
      if(calle<0 || calle>=numCalles || cerrada[calle]) return false;
      cerrada[calle] = true;                        // Se cierra de prueba...
      bool ok = todoConectado();                    // ...se revisa con Dijkstra...
      cerrada[calle] = false;                       // ...y se vuelve a abrir
      origenCalculado = -1;
      return ok;
    };

    int costoCalle(int calle){                      // Costo actual de una calle
      return costo[calle];
    };


    // ---------- DIJKSTRA ----------

    void calcularRutas(int origen){                 // Dijkstra: calcula la distancia más corta desde "origen" a TODOS los nodos → O((V+E) log V)
      if(origen<0 || origen>=numNodos) return;
      bool visitado[MAX_NODOS];                     // visitado[i] = ya se fijó la distancia definitiva del nodo i
      for(int i=0; i<numNodos; i++){                // Al inicio no se conoce ningún camino
        dist[i] = INFINITO;                         // Distancia infinita
        previo[i] = -1;                             // Sin nodo anterior
        visitado[i] = false;
      };
      MonticuloMax cola;           // Montículo máximo con claves = distancias NEGATIVAS: sale siempre la menor distancia
      dist[origen] = 0;                             // Llegar al origen cuesta 0
      cola.insertar(ElementoMonticulo(origen, -0));
      while(!cola.esvacia()){                       // Mientras queden nodos por revisar
        ElementoMonticulo e;
        cola.extraer(e);                            // Sale el nodo más cercano de los pendientes
        int u = e.dato;
        if(visitado[u]) continue;                   // Entrada vieja: ese nodo ya se había fijado con una distancia menor (el montículo no tiene "decrementar clave")
        visitado[u] = true;                         // Su distancia ya es la definitiva
        Arista * a = ady[u];                        // Se recorren las calles que salen de u
        while(a != NULL){
          if(cerrada[a->calle]){ a = a->sig; continue; };      // Calle cerrada por Jonatan: no se puede pasar
          int v = a->destino;                       // Nodo vecino
          int nueva = dist[u] + costo[a->calle];    // Distancia a v pasando por u
          if(nueva < dist[v]){                      // Si este camino es mejor que el que se conocía
            dist[v] = nueva;                        // Se actualiza la distancia
            previo[v] = u;                          // Y se anota por dónde se llegó (para reconstruir la ruta)
            cola.insertar(ElementoMonticulo(v, -nueva));       // Entra al montículo con la nueva distancia (negativa)
          };
          a = a->sig;                               // Siguiente calle
        };
      };
      origenCalculado = origen;                     // Se recuerda desde dónde se calculó
    };

    int distanciaA(int destino){                    // Costo de la ruta más corta (INFINITO si no hay camino). Antes hay que llamar calcularRutas()
      if(destino<0 || destino>=numNodos) return INFINITO;
      return dist[destino];
    };

    int tiempoViaje(int destino){                   // Duración del domicilio: ida y regreso = 2 × el costo de la ruta (INFINITO si no hay camino)
      int d = distanciaA(destino);
      if(d >= INFINITO) return INFINITO;
      return 2 * d;
    };

    bool rutaA(int destino, int ruta[], int & largo){       // Llena "ruta" con los nodos del camino desde el origen hasta el destino, en orden. "largo" = cuántos nodos
      largo = 0;
      if(destino<0 || destino>=numNodos) return false;
      if(origenCalculado == -1 || dist[destino] >= INFINITO) return false;     // No se ha calculado o no hay camino
      int temporal[MAX_NODOS];                      // Se reconstruye de atrás hacia adelante (del destino al origen)
      int n = 0;
      int actual = destino;
      while(actual != -1){                          // El origen tiene previo = -1
        temporal[n] = actual;
        n++;
        actual = previo[actual];
      };
      for(int i=0; i<n; i++){                       // Se invierte para que quede del origen al destino
        ruta[i] = temporal[n-1-i];
      };
      largo = n;
      return true;
    };

    // ---------- CONSULTAS ----------

    int cantidadNodos(){ return numNodos; };                    // Cuántos nodos hay
    int cantidadCalles(){ return numCalles; };                  // Cuántas calles hay
    std::string nombreNodo(int i){ return nombres[i]; };        // Nombre de un nodo (para dibujarlo)
    int extremoUno(int calle){ return extremoA[calle]; };       // Un extremo de la calle (para dibujarla)
    int extremoDos(int calle){ return extremoB[calle]; };       // El otro extremo de la calle

    int costoEntre(int a, int b){                   // Costo de la calle que une a con b (-1 si no hay calle directa)
      if(a<0 || a>=numNodos) return -1;
      Arista * aux = ady[a];
      while(aux != NULL){
        if(aux->destino == b) return costo[aux->calle];
        aux = aux->sig;
      };
      return -1;
    };

    int calleEntre(int a, int b){                   // Número de la calle que une a con b (-1 si no hay calle directa)
      if(a<0 || a>=numNodos) return -1;
      Arista * aux = ady[a];                        // Se recorre la lista de adyacencia de a
      while(aux != NULL){
        if(aux->destino == b) return aux->calle;
        aux = aux->sig;
      };
      return -1;
    };

  protected:

    void armarCiudad(){                             // Crea la ciudad fija: 1 restaurante + 4 intersecciones + 6 casas
      agregarNodo("Restaurante");                   // Nodo 0
      agregarNodo("Esquina A");                     // Nodos 1 a 4: intersecciones
      agregarNodo("Esquina B");
      agregarNodo("Esquina C");
      agregarNodo("Esquina D");
      agregarNodo("Casa 1");                        // Nodos 5 a 10: casas
      agregarNodo("Casa 2");
      agregarNodo("Casa 3");
      agregarNodo("Casa 4");
      agregarNodo("Casa 5");
      agregarNodo("Casa 6");
      agregarCalle(0, 1, 1);                        // Restaurante con las esquinas A y B
      agregarCalle(0, 2, 1);
      agregarCalle(1, 2, 1);                        // Calles entre esquinas (forman un circuito: así hay rutas alternativas)
      agregarCalle(1, 3, 1);
      agregarCalle(2, 4, 1);
      agregarCalle(3, 4, 1);
      agregarCalle(5, 1, 1);                        // Casas conectadas a las esquinas (algunas con dos calles)
      agregarCalle(6, 1, 1);
      agregarCalle(6, 3, 1);
      agregarCalle(7, 2, 1);
      agregarCalle(8, 2, 1);
      agregarCalle(8, 4, 1);
      agregarCalle(9, 3, 1);
      agregarCalle(10, 4, 1);
      agregarCalle(10, 3, 1);
    };

    Arista * ady[MAX_NODOS];          // Lista de adyacencia: ady[i] = primera calle que sale del nodo i
    std::string nombres[MAX_NODOS];   // Nombre de cada nodo
    int numNodos;                     // Cuántos nodos hay
    int extremoA[MAX_CALLES];         // Un extremo de cada calle
    int extremoB[MAX_CALLES];         // El otro extremo de cada calle
    int costo[MAX_CALLES];            // Costo en ticks de cada calle
    bool cerrada[MAX_CALLES];         // true = Jonatan la cerró esta noche
    bool lenta[MAX_CALLES];           // true = el ataque Tráfico le subió el costo esta noche
    int numCalles;                    // Cuántas calles hay
    int dist[MAX_NODOS];              // Resultado de Dijkstra: distancia más corta desde el origen a cada nodo
    int previo[MAX_NODOS];            // Resultado de Dijkstra: nodo anterior en la ruta más corta
    int origenCalculado;              // Origen del último Dijkstra (-1 si las distancias ya no sirven)
};

#endif                    // Fin de la protección contra lecturas repetidas
