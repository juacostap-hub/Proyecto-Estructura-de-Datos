#ifndef JUEGO_H           // Evita que este archivo se lea más de una vez al compilar
#define JUEGO_H           // Marca que este archivo ya fue leído

// JUEGO "RESTAURANT RUSH": toda la lógica de la partida
// El tiempo avanza por TICKS: cada acción del jugador cuesta 1 tick y después se llama a avanzarTick(),
// que hace llegar clientes, baja la paciencia, avanza la cocina, etc. No se usan hilos.
// De noche, avanzarTick() mueve el reloj de la noche: cada calle que recorre la moto cuesta tantos ticks como su costo.
//
// Estructuras de datos que usa (todas hechas a mano):
//   ArregloDinamico   → menú, mesas, cuentas pendientes, registro de ventas, historial, domicilios, ranking, mensajes
//   MatrizDispersa    → mapa del restaurante (solo se guardan las casillas ocupadas)
//   MonticuloMax      → pedidos de la cocina (clave = -tiempo límite: sale primero el más urgente)
//   Grafo + Dijkstra  → ciudad de la noche y rutas de los domicilios
//   Cola y ColaPrioridad (fase 2) → fila de espera común y fila VIP

#include <string>
#include "Restaurante.h"      // Reglas y clases de datos
#include "ArregloDinamico.h"  // Arreglo dinámico (plantilla) de la Persona 1
#include "MatrizDispersa.h"   // Matriz dispersa de la Persona 1
#include "MonticuloMax.h"     // Montículo máximo
#include "Grafo.h"            // Grafo de la ciudad + Dijkstra
#include "ColaComun.h"        // Cola normal de la fase 2
#include "ColaVIP.h"          // Cola con prioridad de la fase 2

enum Fase { FASE_INICIO = 0, FASE_DIA, FASE_NOCHE, FASE_TIENDA, FASE_FINAL };   // En qué pantalla está el juego
enum EstadoNoche { NOCHE_EN_BASE = 0, NOCHE_DESVIO };    // De noche: moto en el restaurante, o detenida por una calle cerrada

struct Resultado {                    // Lo que devuelve cada acción del jugador
    bool ok;                          // true = se hizo; false = no se pudo (el mensaje dice por qué)
    std::string mensaje;              // Texto para mostrar
    Resultado() : ok(false), mensaje("") {}
    Resultado(bool _ok, std::string _mensaje) : ok(_ok), mensaje(_mensaje) {}
};

class Juego {
public:
    Juego(bool usarArchivoRanking = true);                // Crea el juego en la pantalla de inicio y carga el ranking (las pruebas no usan el archivo)

    // ---------- Acciones del jugador (cada una devuelve si se pudo y un mensaje) ----------
    Resultado nuevaPartida(const std::string& nombre, unsigned semilla);    // Empieza una partida nueva
    Resultado mover(char direccion);                      // 'A' arriba, 'B' abajo, 'I' izquierda, 'D' derecha (1 tick)
    Resultado interactuar();                              // Tecla E: cocina, mesa o caja vecina (1 tick)
    Resultado esperar();                                  // Deja pasar 1 tick
    Resultado sentar(int idMesa);                         // Sienta al grupo que sigue en la mesa elegida (1 tick)
    Resultado cerrarTemprano();                           // Termina el día si ya no llegan clientes y el salón está vacío
    Resultado salir(int idA, int idB, const int ruta[], int largo);   // Noche: sale con 1 o 2 pedidos (idB = -1) por la ruta que trazó el jugador
    Resultado desviar(const int ruta[], int largo);       // Noche: nueva ruta desde la esquina donde Jonatan cerró la calle
    Resultado verRuta(int idA, int idB);                  // Noche: el GPS muestra la ruta más rápida (ese viaje ya no da bono)
    Resultado responderUrgente(bool acepta);              // Noche: acepta o rechaza el pedido urgente
    Resultado terminarNoche();                            // Noche: termina con los domicilios que falten
    Resultado comprarMejora(int tipo);                    // Tienda: compra una mejora
    Resultado siguienteJornada();                         // Tienda: empieza la siguiente jornada

    std::string estadoJSON();                             // Todo el estado del juego en JSON (para la página)

    // ---------- Consultas que usa el jugador automático de prueba ----------
    int fase() const { return faseActual; }
    int jornadaActual() const { return jornada; }
    int tickActual() const { return tick; }
    int dineroActual() const { return dinero; }
    int reputacionActual() const { return reputacion; }
    bool gano() const { return victoria; }
    int puntaje() const;
    int jugadorFila() const { return jugadorF; }
    int jugadorCol() const { return jugadorC; }
    bool esTransitable(int f, int c) const { return mapa.esTransitable(f, c); }
    int mesaDelPlatoEnMano() const { return manoMesa; }   // -1 si no carga nada
    bool cocinaLibre() const { return cocinaCliente == -1; }
    bool platoListo() const { return cocinaCliente != -1 && tick >= cocinaListaEn; }
    int pedidosEnEspera() { return pedidos.longitud(); }
    int cuentasPendientes() const { return cuentas.tamano(); }
    int mesaParaSiguiente();                              // Mesa libre más pequeña donde cabe el grupo que sigue (-1 si no hay)
    bool tieneMejora(int tipo) const { return mejoras[tipo]; }
    bool comproHoy() const { return comproEstaJornada; }
    int cantidadDomicilios() const { return domicilios.tamano(); }
    Domicilio domicilio(int i) const { return domicilios.obtener(i); }
    int tickNocheActual() const { return tickNoche; }
    int duracionNocheActual() const { return duracionNoche; }
    int duracionViaje(int idA, int idB);                  // Ticks de la ruta más rápida para entregar 1 o 2 domicilios (-1 si no se puede)
    bool rutaOptima(int idA, int idB, int ruta[], int& largo);    // Ruta más rápida desde donde está la moto (en desvío usa lo que lleva)
    int costoCalle(int a, int b);                         // Costo de la calle abierta entre a y b (-1 si no hay)
    int posicionMoto() const { return posMoto; }
    bool enDesvio() const { return faseActual == FASE_NOCHE && estadoNoche == NOCHE_DESVIO; }
    bool hayOferta() const { return hayOfertaUrgente; }
    Domicilio oferta() const { return ofertaUrgente; }
    int estrellas() const { return estrellasFinal; }
    int puestoEnRanking() const { return puestoRanking; }
    ResumenJornada resumenHoy() const { return hoy; }
    int ataquesHechos() const { return numAtaques; }

private:
    // ---------- Pasos internos ----------
    void avanzarTick();                                   // Lo que pasa en cada tick del día
    void iniciarDia();                                    // Prepara el día de la jornada actual
    void terminarDia();                                   // Cierra el día y prepara la noche
    void terminarJornada();                               // Cierra la noche: tienda o final
    bool validarRuta(const int ruta[], int largo, int desde, std::string& error);   // ¿Calles abiertas, empieza en "desde" y termina en el restaurante?
    int costoDeRuta(const int ruta[], int largo);         // Suma de los costos de las calles de la ruta
    bool rutaVisita(const int ruta[], int largo, int casa) const;  // ¿La ruta pasa por esa casa?
    int optimoDesde(int desde, const int casas[], int n); // Ticks de la ruta más rápida desde "desde", pasando por las casas y volviendo
    bool armarRutaOptima(int desde, const int casas[], int n, int ruta[], int& largo);   // Esa ruta, nodo por nodo (Dijkstra)
    int casasPendientesEnMoto(int casas[]);               // Casas de los pedidos que lleva la moto y no ha entregado
    int limiteDeCasa(int casa) const;                     // Tiempo límite del pedido pendiente de esa casa
    void viajar();                                        // La moto recorre la ruta calle por calle (o se detiene si Jonatan cierra una)
    void llegarA(int nodo);                               // Entrega los pedidos de esa casa
    void terminarViaje();                                 // La moto volvió al restaurante
    void ofrecerUrgente();                                // Aparece el pedido urgente (una vez por noche)
    bool quedanPendientes() const;                        // ¿Quedan domicilios sin entregar?
    void armarMapa();                                     // Llena la matriz dispersa con paredes, mesas, cocina, caja y puerta
    void llegaGrupo();                                    // Un grupo llega a la fila (si cabe)
    void revisarFila();                                   // Los que se cansaron de esperar se van
    bool siguienteCliente(Cliente& c, bool& deVip);       // Quién sigue en la fila (regla de la fase 2) sin sacarlo
    void subirAmenaza(int cantidad);                      // Suma amenaza y, si llega a 100, Jonatan ataca
    void atacar();                                        // Jonatan lanza un ataque
    void cambiarReputacion(int cambio);                   // Suma o resta reputación (entre 0 y 100) y revisa si se perdió
    void decirJonatan(const std::string& frase);          // Lo que dice Jonatan en la esquina de la pantalla
    void anotar(const std::string& evento);               // Agrega un mensaje al registro de eventos
    void finalizar(bool completo);                        // Termina la partida: calcula si ganó y guarda el ranking
    void seFueDeLaFila(const Cliente& c);                 // Castigo y mensaje cuando un grupo se cansa de esperar
    int buscarMesa(int idMesa) const;                     // Posición de la mesa en el arreglo (-1 si no existe)
    int mesaDeCliente(int idCliente) const;               // Posición de la mesa donde está sentado ese grupo (-1 si se fue)
    int mesaVecina() const;                               // Posición de una mesa vecina al jugador (-1 si no hay)
    bool vecinoDe(int f, int c) const;                    // ¿El jugador está al lado de la casilla (f, c)?
    int azar(int minimo, int maximo);                     // Número al azar entre minimo y maximo (incluidos)
    void cargarRanking();                                 // Lee ranking.txt
    void guardarRanking();                                // Escribe ranking.txt

    // ---------- Estado de la partida ----------
    int faseActual;                   // Pantalla actual
    int jornada;                      // 1 a 5
    int tick;                         // Tick del día (0 a 150)
    int dinero;                       // Dinero en caja
    int reputacion;                   // 0 a 100
    int amenaza;                      // 0 a 100
    int nochesPerfectas;              // Noches con todos los domicilios a tiempo
    bool victoria;                    // Al final: true si ganó
    std::string nombreJugador;        // Para el ranking

    // ---------- Día ----------
    MatrizDispersa mapa;              // Mapa del restaurante (matriz dispersa)
    int jugadorF, jugadorC;           // Posición del jugador
    ArregloDinamico<Plato> menu;      // Menú: crece al desbloquear productos
    ArregloDinamico<Mesa> mesas;      // Las 6 mesas
    Cola filaComun;                   // Fila de clientes comunes (cola normal)
    ColaPrioridad filaVIP;            // Fila VIP (cola con prioridad: 3 niveles)
    int vipSeguidos;                  // VIP atendidos seguidos (regla: 2 VIP por cada común)
    int siguienteTurno;               // Número que recibe el próximo grupo
    int proximaLlegada;               // Tick en que llega el próximo grupo
    MonticuloMax pedidos;             // Pedidos esperando cocina: dato = grupo, clave = -tiempo límite
    int cocinaCliente;                // Grupo cuyo plato se está preparando (-1 = cocina libre)
    int cocinaListaEn;                // Tick en que ese plato queda listo
    int manoCliente, manoMesa;        // Plato que carga el jugador (-1 = manos libres)
    int meseroCliente, meseroMesa;    // Plato que lleva el mesero (mejora); -1 = libre
    int meseroLlegaEn, meseroLibreEn; // Cuándo llega a la mesa y cuándo vuelve a estar libre
    ArregloDinamico<Cuenta> cuentas;  // Cuentas pendientes de cobro
    ArregloDinamico<Venta> ventas;    // Registro de ventas de toda la partida
    ResumenJornada hoy;               // Cómo va la jornada actual

    // ---------- Noche ----------
    Grafo ciudad;                     // Ciudad (grafo fijo) con costos que cambian cada jornada
    ArregloDinamico<Domicilio> domicilios;   // Domicilios de la noche
    int tickNoche;                    // Ticks usados en la noche
    int duracionNoche;                // 60 + los ticks que sobraron del día
    int traficoPendiente;             // Ataques de Tráfico que se aplican en la noche
    int ultimaRuta[MAX_RUTA];         // Ruta que mostró el GPS (para dibujarla)
    int largoUltimaRuta;
    int estadoNoche;                  // En el restaurante o en desvío
    int posMoto;                      // Nodo donde está la moto
    int plan[MAX_RUTA];               // Ruta que está siguiendo la moto
    int largoPlan, idxPlan;           // Cuántos nodos tiene y en cuál va
    int carga[2], numCarga;           // Pedidos que lleva la moto en este viaje
    int pasosViaje;                   // Calles recorridas en este viaje
    int costoViaje;                   // Ticks que lleva el viaje
    int optimoViaje;                  // Ticks de la ruta del GPS al salir
    bool viajePerfecto;               // true = cada ruta trazada fue tan rápida como la del GPS
    bool huboCierreEnViaje;           // true = Jonatan cerró una calle en este viaje
    bool gpsVisto;                    // true = el jugador miró la ruta del GPS (este viaje no da bono)
    int recorrido[MAX_RUTA * 2];      // Nodos que recorrió la moto en la última acción (para animarla)
    int largoRecorrido;
    int entregaId[4], entregaPaso[4], numEntregasRecientes;   // Pedidos entregados en la última acción y en qué paso del recorrido
    int cierresRestantes;             // Calles que Jonatan todavía puede cerrar esta noche
    int proximoCierre;                // Desde qué tick de la noche puede cerrar la siguiente
    int calleCerradaAhora;            // Calle que cerró en la última acción (-1 = ninguna)
    bool urgenteOfrecido;             // Ya apareció el pedido urgente esta noche
    bool hayOfertaUrgente;            // Hay un pedido urgente esperando respuesta
    Domicilio ofertaUrgente;          // El pedido urgente ofrecido
    bool hayUltimoViaje;              // Para mostrar cómo le fue en el último viaje
    int ultimoCosto, ultimoOptimo, ultimoBono;
    bool ultimoPerfecto, ultimoConCierre;
    std::string mensajeViaje;         // Lo que pasó en el viaje (se le muestra al jugador)

    // ---------- Mejoras, Jonatan, historial ----------
    bool mejoras[NUM_MEJORAS];        // Qué mejoras se compraron
    bool comproEstaJornada;           // Máximo una mejora por jornada
    std::string fraseJonatan;         // Lo último que dijo Jonatan
    std::string ultimoAtaque;         // Nombre del último ataque ("" si no ha atacado)
    ArregloDinamico<std::string> eventos;        // Últimos mensajes del juego
    ArregloDinamico<ResumenJornada> historial;   // Resumen de cada jornada terminada
    ArregloDinamico<Puntaje> ranking;            // Los 5 mejores puntajes
    bool usaArchivo;                             // false = no lee ni escribe ranking.txt (pruebas)
    int estrellasFinal;                          // Estrellas de la reseña final (1 a 5)
    int puestoRanking;                           // Puesto en el ranking al terminar (0 = no entró)
    int numAtaques;                              // Ataques que ha hecho Jonatan (la página anima cada uno nuevo)
};

#endif                    // Fin de la protección contra lecturas repetidas
