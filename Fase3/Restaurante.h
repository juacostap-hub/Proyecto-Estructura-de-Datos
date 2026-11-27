#ifndef RESTAURANTE_H     // Evita que este archivo se lea más de una vez al compilar
#define RESTAURANTE_H     // Marca que este archivo ya fue leído

// REGLAS Y DATOS DEL JUEGO "RESTAURANT RUSH"
// Aquí están todos los números del juego (calibrados con la simulación del documento de reglas)
// y las clases sencillas que guardan los datos: platos, mesas, cuentas, domicilios, etc.
// Plato y Mesa vienen del notebook de la Persona 1 (Proyecto2ED.cpp) y se ampliaron para el juego.

#include <string>         // Para usar texto (std::string)


// ---------------- TIEMPO ----------------
const int TICKS_DIA       = 150;      // Duración del día en ticks (cada acción del jugador = 1 tick)
const int TICKS_NOCHE     = 60;       // Duración base de la noche (se le suman los ticks que sobren del día)
const int ULTIMA_LLAMADA  = 25;       // En los últimos 25 ticks del día ya no llegan clientes
const int TICKS_COMIENDO  = 3;        // Cuánto tarda un grupo en comer después de recibir su plato

// ---------------- FILA Y PACIENCIA ----------------
const int PACIENCIA_FILA  = 35;       // Ticks que un grupo espera en la fila antes de irse
const int FILA_MAXIMA     = 4;        // Si ya hay 4 grupos esperando, los que llegan siguen de largo
const int AMBIENTE_EXTRA  = 4;        // Ticks de paciencia que suma la mejora "Mejor ambiente"

// ---------------- REPUTACIÓN Y META ----------------
const int REP_INICIAL     = 75;       // Reputación con la que se empieza
const int REP_MAXIMA      = 100;      // La reputación nunca pasa de 100
const int REP_META        = 40;       // Para ganar: reputación de 40 o más al final...
const int DINERO_META     = 1200000;  // ...y $1.200.000 o más en caja
const int CASTIGO_MESA    = 6;        // Un grupo se va de la mesa sin comer
const int CASTIGO_FILA    = 2;        // Un grupo se cansa de esperar en la fila
const int CASTIGO_TARDE   = 3;        // Un domicilio llega tarde
const int CASTIGO_SIN_ENTREGAR = 5;   // Un domicilio queda sin entregar al acabar la noche

// ---------------- AMENAZA DE JONATAN ----------------
const int AMENAZA_MESA    = 10;       // Sube cuando un grupo se va de la mesa
const int AMENAZA_FILA    = 5;        // Sube cuando un grupo se va de la fila
const int AMENAZA_MITAD   = 5;        // Sube cuando un pedido llega a la mitad de su paciencia sin cocinarse
const int AMENAZA_TARDE   = 5;        // Sube cuando un domicilio llega tarde
const int AMENAZA_MAXIMA  = 100;      // Al llegar a 100, Jonatan ataca...
const int AMENAZA_TRAS_ATAQUE = 30;   // ...y la barra baja a 30

// ---------------- DINERO ----------------
const int PAGO_A_TIEMPO   = 15000;    // Domicilio a tiempo
const int PAGO_POR_TICK   = 1000;     // Más $1.000 por cada tick que sobre al entregar
const int PAGO_TARDE      = 7500;     // Domicilio tarde: la mitad
const int COSTO_AUDITORIA = 5000;     // Ataque Auditoría: cuesta esto cada cuenta sin cobrar
const int REP_AUDITORIA   = 2;        // Ataque Auditoría: y esta reputación por cada cuenta
const int LIMITE_DOM_MIN  = 15;       // Tiempo límite de un domicilio: entre 15...
const int LIMITE_DOM_MAX  = 45;       // ...y 45 ticks desde que empieza la noche
const int BONO_NOCHE_PERFECTA = 50;   // Puntos extra por cada noche con todos los domicilios a tiempo

// ---------------- NOCHE: RUTAS PROPIAS, GPS, CALLES CERRADAS Y PEDIDO URGENTE ----------------
const int BONO_RUTA_PERFECTA  = 5000; // Viaje con una ruta igual de rápida que la del GPS (sin haberlo mirado)
const int MAX_RUTA            = 40;   // Máximo de lugares en una ruta trazada por el jugador
const int CIERRE_PRIMERO_MIN  = 4;    // Jonatan cierra la primera calle desde el tick 4...
const int CIERRE_PRIMERO_MAX  = 16;   // ...hasta el 16 de la noche (si el jugador va de viaje)
const int CIERRE_SEPARACION_MIN = 8;  // Entre un cierre y el siguiente pasan de 8...
const int CIERRE_SEPARACION_MAX = 16; // ...a 16 ticks
const int URGENTE_DESDE       = 2;    // El pedido urgente aparece desde la jornada 2 (una vez por noche)
const int PAGO_URGENTE        = 30000;// Pedido urgente a tiempo: $30.000...
const int PAGO_URGENTE_TICK   = 2000; // ...más $2.000 por cada tick que sobre
const int PAGO_URGENTE_TARDE  = 15000;// Pedido urgente tarde: la mitad
const int HOLGURA_URGENTE_MIN = 2;    // Su límite es: tick actual + ida más corta + 2...
const int HOLGURA_URGENTE_MAX = 5;    // ...a 5 ticks de holgura

// ---------------- TIPOS DE CLIENTE (el índice es la tarjeta de la fase 2) ----------------
// 0 = común, 1 = frecuente, 2 = amigo del gerente, 3 = prestigio
const int PAGO_TIPO[4]      = {100, 125, 150, 200};    // Cuánto paga, en porcentaje del precio normal
const int PACIENCIA_TIPO[4] = {100,  90,  80,  70};    // Cuánta paciencia tiene, en porcentaje de la de la jornada

// ---------------- JORNADAS ----------------
struct Jornada {                      // Cómo es cada una de las 5 jornadas
    const char* nombre;               // Nombre que se muestra
    int llegada;                      // Llega un grupo cada "llegada" ticks (±2 al azar)
    int paciencia;                    // Paciencia en la mesa de un cliente común
    int vip;                          // Porcentaje de grupos que son VIP
    int domicilios;                   // Cuántos domicilios hay en la noche
    int ataques;                      // Cuántos ataques puede usar Jonatan (0 = solo observa)
    int cierres;                      // Cuántas calles puede cerrar Jonatan en la noche
};

const int NUM_JORNADAS = 5;
const Jornada JORNADAS[NUM_JORNADAS] = {
    {"Apertura",        16, 36, 30, 2, 0, 0},
    {"Hora pico",       15, 34, 35, 3, 1, 1},
    {"Noche lluviosa",  14, 32, 40, 3, 2, 1},
    {"Promociones",     12, 30, 45, 4, 3, 1},
    {"Gran final",      11, 28, 50, 5, 4, 2}
};

// ---------------- ATAQUES (en el orden en que se van desbloqueando) ----------------
enum Ataque { AVALANCHA = 0, TRAFICO, CAOS, AUDITORIA };
const int GRUPOS_AVALANCHA   = 3;     // Avalancha: llegan 3 grupos de golpe
const int CALLES_TRAFICO     = 3;     // Tráfico: 3 calles...
const int AUMENTO_TRAFICO    = 4;     // ...suben 4 ticks esa noche
const int OBSTACULOS_CAOS    = 3;     // Caos: 3 obstáculos...
const int TICKS_OBSTACULO    = 20;    // ...durante 20 ticks

// ---------------- MEJORAS ----------------
enum TipoMejora { MEJORA_MESERO = 0, MEJORA_COCINA, MEJORA_MOTO, MEJORA_AMBIENTE, NUM_MEJORAS };
const int PRECIO_MEJORA[NUM_MEJORAS] = {180000, 100000, 90000, 80000};
const char* const NOMBRE_MEJORA[NUM_MEJORAS] = {"Mesero", "Mejor cocina", "Moto mejorada", "Mejor ambiente"};

// ---------------- POSICIONES DEL MAPA (fila, columna) ----------------
const int FILAS_MAPA = 10, COLUMNAS_MAPA = 15;          // Mapa de 15 columnas × 10 filas
const int COCINA_F = 0, COCINA_C = 7;                   // La cocina está en la pared de arriba
const int CAJA_F   = 0, CAJA_C   = 11;                  // La caja, junto a la cocina
const int PUERTA_F = 9, PUERTA_C = 7;                   // La puerta, en la pared de abajo
const int INICIO_F = 5, INICIO_C = 7;                   // Donde empieza el jugador cada día


// PLATO DEL MENÚ (de la Persona 1)
class Plato {
public:
    std::string nombre;               // Nombre del producto
    int precio;                       // Precio por persona, en pesos
    int ticksPreparacion;             // Cuántos ticks tarda la cocina
    int jornadaDisponible;            // Desde qué jornada aparece en el menú

    Plato() : nombre(""), precio(0), ticksPreparacion(1), jornadaDisponible(1) {}
    Plato(std::string _nombre, int _precio, int _ticks, int _jornada)
        : nombre(_nombre), precio(_precio), ticksPreparacion(_ticks), jornadaDisponible(_jornada) {}
};

// Los 10 productos del juego. El menú (arreglo dinámico) los va recibiendo a medida que se desbloquean.
const int NUM_PRODUCTOS = 10;
const Plato PRODUCTOS[NUM_PRODUCTOS] = {
    Plato("Café",          4000, 1, 1),
    Plato("Limonada",      5000, 1, 1),
    Plato("Jugo natural",  6000, 1, 1),
    Plato("Papas fritas",  8000, 2, 1),
    Plato("Sándwich",     12000, 2, 1),
    Plato("Hamburguesa",  20000, 3, 1),
    Plato("Ensalada",     14000, 2, 2),
    Plato("Postre",       10000, 2, 2),
    Plato("Pasta",        18000, 4, 3),
    Plato("Pizza",        30000, 5, 3)
};


// MESA (de la Persona 1, ampliada con lo que necesita el juego por ticks)
class Mesa {
public:
    int id;                           // Número de la mesa (1 a 6)
    int capacidad;                    // Cuántas personas caben
    int fila, col;                    // Dónde está en el mapa
    bool ocupada;                     // true = hay un grupo sentado
    int idCliente;                    // Número del grupo sentado (su turno)
    int personas;                     // Cuántas personas son
    int tarjeta;                      // Tipo de cliente: 0 común, 1 a 3 VIP
    int plato;                        // Posición del plato pedido en el menú
    int tickSentado;                  // Tick en que se sentó
    int tickLimite;                   // Tick en que se va si no le han servido
    int comiendoHasta;                // Tick en que termina de comer (-1 = todavía no le sirven)
    int cuenta;                       // Lo que va a pagar el grupo
    bool cocinando;                   // true = su pedido ya salió del montículo hacia la cocina
    bool avisoMitad;                  // true = ya subió la amenaza por llegar a la mitad de su paciencia

    Mesa() : id(0), capacidad(2), fila(0), col(0) { liberar(); }
    Mesa(int _id, int _capacidad, int _fila, int _col) : id(_id), capacidad(_capacidad), fila(_fila), col(_col) { liberar(); }

    void liberar() {                  // Deja la mesa vacía y lista para otro grupo
        ocupada = false;
        idCliente = -1;
        personas = 0;
        tarjeta = 0;
        plato = -1;
        tickSentado = 0;
        tickLimite = 0;
        comiendoHasta = -1;
        cuenta = 0;
        cocinando = false;
        avisoMitad = false;
    }
};

// Las 6 mesas del mapa: 3 de 2 personas y 3 de 4
const int NUM_MESAS = 6;
const int MESAS_INICIALES[NUM_MESAS][4] = {     // {id, capacidad, fila, columna}
    {1, 2, 3, 2}, {2, 4, 3, 5}, {3, 4, 3, 9}, {4, 2, 3, 12}, {5, 4, 6, 3}, {6, 2, 6, 11}
};

// Casillas donde el ataque Caos puede poner obstáculos: están separadas entre sí y nunca encierran
// una mesa ni tapan la cocina, la caja o la puerta.
const int NUM_CASILLAS_CAOS = 10;
const int CASILLAS_CAOS[NUM_CASILLAS_CAOS][2] = {
    {2, 10}, {4, 4}, {4, 10}, {5, 6}, {5, 8}, {7, 2}, {7, 5}, {7, 9}, {7, 12}, {8, 4}
};


class Cuenta {                        // Una cuenta que dejó un grupo y todavía no se ha cobrado
public:
    int mesa;                         // De qué mesa es
    int monto;                        // Cuánto es
    Cuenta() : mesa(0), monto(0) {}
    Cuenta(int _mesa, int _monto) : mesa(_mesa), monto(_monto) {}
};

class Venta {                         // Una línea del registro de ventas
public:
    int jornada;                      // En qué jornada
    std::string detalle;              // Qué se vendió (por ejemplo "Mesa 3" o "Domicilio a Casa 2")
    int monto;                        // Cuánto entró
    Venta() : jornada(0), detalle(""), monto(0) {}
    Venta(int _jornada, std::string _detalle, int _monto) : jornada(_jornada), detalle(_detalle), monto(_monto) {}
};

enum EstadoDomicilio { DOM_PENDIENTE = 0, DOM_A_TIEMPO, DOM_TARDE };

class Domicilio {                     // Un pedido a domicilio de la noche
public:
    int id;                           // Número del pedido (1, 2, 3...)
    int casa;                         // Nodo del grafo al que va (5 a 10)
    int limite;                       // Tick de la noche en que debe llegar
    int estado;                       // Pendiente, entregado a tiempo o tarde
    int llegada;                      // Tick en que llegó (si ya se entregó)
    bool urgente;                     // true = es el pedido urgente de la noche (paga el doble)
    int pago;                         // Lo que pagó al entregarlo
    Domicilio() : id(0), casa(0), limite(0), estado(DOM_PENDIENTE), llegada(0), urgente(false), pago(0) {}
    Domicilio(int _id, int _casa, int _limite, bool _urgente = false)
        : id(_id), casa(_casa), limite(_limite), estado(DOM_PENDIENTE), llegada(0), urgente(_urgente), pago(0) {}
};

class ResumenJornada {                // Cómo le fue al jugador en una jornada (historial)
public:
    int jornada;
    int atendidos;                    // Grupos que comieron
    int perdidos;                     // Grupos que se fueron (de la fila o de la mesa)
    int ventasDia;                    // Dinero cobrado en la caja
    int perdidoSinCobrar;             // Dinero de cuentas que no se cobraron antes de cerrar
    int ganadoNoche;                  // Dinero de los domicilios (incluye bonos de ruta perfecta)
    int entregas;                     // Domicilios entregados
    int pedidos;                      // Domicilios de la noche
    int reputacion;                   // Reputación al final de la jornada
    int dinero;                       // Dinero en caja al final de la jornada
    int repInicio;                    // Reputación al abrir el restaurante
    int repFinDia;                    // Reputación al cerrar el restaurante (antes de la noche)
    int aTiempo;                      // Domicilios que llegaron a tiempo
    int viajes;                       // Viajes de la moto
    int rutasPerfectas;               // Viajes con una ruta igual de rápida que la del GPS
    int cierres;                      // Calles que cerró Jonatan
    int urgentes;                     // Pedidos urgentes aceptados
    ResumenJornada() : jornada(0), atendidos(0), perdidos(0), ventasDia(0), perdidoSinCobrar(0),
                       ganadoNoche(0), entregas(0), pedidos(0), reputacion(0), dinero(0),
                       repInicio(0), repFinDia(0), aTiempo(0), viajes(0), rutasPerfectas(0), cierres(0), urgentes(0) {}
};

class Puntaje {                       // Una línea del ranking
public:
    std::string nombre;
    int puntos;
    bool gano;
    Puntaje() : nombre(""), puntos(0), gano(false) {}
    Puntaje(std::string _nombre, int _puntos, bool _gano) : nombre(_nombre), puntos(_puntos), gano(_gano) {}
};

#endif                    // Fin de la protección contra lecturas repetidas
