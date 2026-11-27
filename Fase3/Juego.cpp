// JUEGO "RESTAURANT RUSH": implementación de la lógica (ver Juego.h)

#include "Juego.h"
#include <sstream>        // Para armar el texto JSON (std::ostringstream)
#include <fstream>        // Para leer y guardar el ranking en ranking.txt
#include <cstdlib>        // Para rand(), srand() y abs()

using namespace std;

// ============================================================
// Ayudas pequeñas
// ============================================================

static string pesos(int n) {                       // 1200000 → "$1.200.000"
    string signo = n < 0 ? "-" : "";               // Guarda el signo aparte
    string s = to_string(n < 0 ? -n : n);          // Los dígitos sin signo
    string r;                                      // Resultado con puntos de miles
    int cuenta = 0;                                // Dígitos copiados desde la derecha
    for (int i = (int)s.size() - 1; i >= 0; i--) { // Recorre de derecha a izquierda
        r = s[i] + r;                              // Agrega el dígito adelante
        cuenta++;
        if (cuenta % 3 == 0 && i > 0) r = "." + r; // Cada 3 dígitos, un punto
    }
    return signo + "$" + r;
}

static string comillas(const string& s) {          // Texto entre comillas y seguro para JSON
    string r = "\"";
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"') r += "\\\"";                 // Las comillas se escapan
        else if (c == '\\') r += "\\\\";           // La barra invertida también
        else if (c < 0x20) r += ' ';               // Saltos de línea y otros controles → espacio
        else r += (char)c;
    }
    return r + "\"";
}

static const char* NOMBRE_TIPO[4] = {"Común", "Frecuente", "Amigo del gerente", "Prestigio"};      // Nombre de cada tipo de cliente

// Frases de Jonatan (historia propia: hablan de comida y servicio, nunca de la materia)
static const char* FRASES_MESA[3] = {"Ese pedido lleva demasiado tiempo.", "Anotado: comida fantasma.", "¿Cocinan o están sembrando los ingredientes?"};
static const char* FRASES_FILA[3] = {"¿Nadie atiende la puerta?", "Mi abuela atiende más rápido.", "Esa fila se mueve menos que mi paciencia."};
static const char* FRASES_JORNADA[NUM_JORNADAS] = {"Dicen que aquí se come bien. Eso lo decido yo.", "Hoy vengo con hambre... y con amigos.",
                                                   "Está lloviendo. Veremos si sus motos nadan.", "Promociones, ¿eh? Más gente, más errores.",
                                                   "Última noche. Mi libreta está lista."};
static const char* FRASES_NOCHE[NUM_JORNADAS] = {"A ver si sus domicilios llegan calientes.", "Tengo hambre... y un mapa de la ciudad.",
                                                 "Con esta lluvia, cada calle es un río.", "Promociones de noche. Qué valientes.",
                                                 "Última noche. Conozco cada calle de esta ciudad."};
static const char* FRASES_CIERRE[3] = {"Uy, esa calle está en obras. Qué pena.", "Calle cerrada. Recalcule, por favor.", "Por aquí no pasa nadie. Órdenes mías."};
static const char* FRASES_RUTA[3] = {"Hmm. Ni yo conozco tan bien estas calles.", "Ruta impecable. Lo anoto... a regañadientes.", "¿Tiene un GPS en la cabeza?"};
static const char* FRASES_FINAL[5] = {"Cierren la cocina. Aquí no vuelvo.", "Cierren la cocina. Aunque el café... no estaba mal.",
                                      "Tres estrellas... y estoy siendo generoso.", "Cuatro estrellas. Casi me convencen.",
                                      "Cinco estrellas. No se lo digan a nadie."};
static const char* APELLIDOS[12] = {"Rojas", "Gómez", "Pérez", "Díaz", "Torres", "Ruiz",          // Para nombrar a los grupos
                                    "Vargas", "Castro", "Ortiz", "Molina", "Suárez", "Ríos"};


// ============================================================
// Creación y partida nueva
// ============================================================

Juego::Juego(bool usarArchivoRanking) : mapa(FILAS_MAPA, COLUMNAS_MAPA) {      // El mapa es una matriz dispersa de 10 filas × 15 columnas
    usaArchivo = usarArchivoRanking;
    faseActual = FASE_INICIO;                     // Empieza en la pantalla de inicio
    jornada = 1;
    tick = 0;
    dinero = 0;
    reputacion = REP_INICIAL;
    amenaza = 0;
    nochesPerfectas = 0;
    victoria = false;
    jugadorF = INICIO_F;
    jugadorC = INICIO_C;
    vipSeguidos = 0;
    siguienteTurno = 1;
    proximaLlegada = 0;
    cocinaCliente = -1;
    cocinaListaEn = 0;
    manoCliente = -1;
    manoMesa = -1;
    meseroCliente = -1;
    meseroMesa = -1;
    meseroLlegaEn = 0;
    meseroLibreEn = 0;
    tickNoche = 0;
    duracionNoche = TICKS_NOCHE;
    traficoPendiente = 0;
    largoUltimaRuta = 0;
    estadoNoche = NOCHE_EN_BASE;
    posMoto = RESTAURANTE;
    largoPlan = 0;
    idxPlan = 0;
    carga[0] = carga[1] = -1;
    numCarga = 0;
    pasosViaje = 0;
    costoViaje = 0;
    optimoViaje = 0;
    viajePerfecto = false;
    huboCierreEnViaje = false;
    gpsVisto = false;
    largoRecorrido = 0;
    numEntregasRecientes = 0;
    cierresRestantes = 0;
    proximoCierre = 0;
    calleCerradaAhora = -1;
    urgenteOfrecido = false;
    hayOfertaUrgente = false;
    hayUltimoViaje = false;
    ultimoCosto = ultimoOptimo = ultimoBono = 0;
    ultimoPerfecto = ultimoConCierre = false;
    estrellasFinal = 0;
    puestoRanking = 0;
    numAtaques = 0;
    comproEstaJornada = false;
    for (int i = 0; i < NUM_MEJORAS; i++) mejoras[i] = false;   // Ninguna mejora comprada
    fraseJonatan = "Dicen que aquí se come bien. Eso lo decido yo.";
    cargarRanking();                              // Lee los mejores puntajes guardados
}

Resultado Juego::nuevaPartida(const string& nombre, unsigned semilla) {
    string limpio;                                // Nombre sin caracteres que dañen el archivo del ranking
    for (size_t i = 0; i < nombre.size() && limpio.size() < 20; i++) {
        char c = nombre[i];
        if (c == ';' || c == '\n' || c == '\r') continue;     // ';' separa los campos de ranking.txt
        limpio += c;
    }
    nombreJugador = limpio.empty() ? "Jugador" : limpio;

    srand(semilla);                               // Misma semilla = misma partida (sirve para las pruebas)
    jornada = 1;
    dinero = 0;
    reputacion = REP_INICIAL;
    amenaza = 0;
    nochesPerfectas = 0;
    victoria = false;
    traficoPendiente = 0;
    siguienteTurno = 1;
    comproEstaJornada = false;
    ultimoAtaque = "";
    numAtaques = 0;
    estrellasFinal = 0;
    puestoRanking = 0;
    for (int i = 0; i < NUM_MEJORAS; i++) mejoras[i] = false;
    menu.limpiar();                               // El menú se vuelve a llenar en iniciarDia()
    ventas.limpiar();
    historial.limpiar();
    eventos.limpiar();
    mesas.limpiar();
    for (int i = 0; i < NUM_MESAS; i++) {         // Crea las 6 mesas del mapa
        mesas.agregar(Mesa(MESAS_INICIALES[i][0], MESAS_INICIALES[i][1], MESAS_INICIALES[i][2], MESAS_INICIALES[i][3]));
    }
    decirJonatan("Dicen que aquí se come bien. Eso lo decido yo.");
    iniciarDia();                                 // Arranca la jornada 1
    return Resultado(true, "¡Empieza la jornada 1! Abre el restaurante.");
}


// ============================================================
// El día
// ============================================================

void Juego::armarMapa() {                         // Llena la matriz dispersa: solo las casillas ocupadas
    mapa.limpiarTodo();                           // Borra todo lo anterior (incluidos obstáculos)
    mapa.insertar(COCINA_F, COCINA_C, COCINA);    // Primero los lugares especiales de las paredes
    mapa.insertar(CAJA_F, CAJA_C, CAJA);
    mapa.insertar(PUERTA_F, PUERTA_C, PUERTA);
    for (int c = 0; c < COLUMNAS_MAPA; c++) {     // Paredes de arriba y de abajo
        mapa.insertar(0, c, PARED);               // (insertar no hace nada si la casilla ya está ocupada)
        mapa.insertar(FILAS_MAPA - 1, c, PARED);
    }
    for (int f = 1; f < FILAS_MAPA - 1; f++) {    // Paredes de los lados
        mapa.insertar(f, 0, PARED);
        mapa.insertar(f, COLUMNAS_MAPA - 1, PARED);
    }
    for (int i = 0; i < mesas.tamano(); i++) {    // Las mesas (idObjeto = número de la mesa)
        const Mesa& m = mesas[i];
        mapa.insertar(m.fila, m.col, MESA, m.id);
    }
}

void Juego::iniciarDia() {
    faseActual = FASE_DIA;
    tick = 0;
    for (int i = 0; i < NUM_PRODUCTOS; i++) {     // El menú (arreglo dinámico) crece con los productos de esta jornada
        if (PRODUCTOS[i].jornadaDisponible == jornada) {
            menu.agregar(PRODUCTOS[i]);
            if (jornada > 1) anotar("Nuevo en el menú: " + PRODUCTOS[i].nombre + ".");
        }
    }
    armarMapa();
    jugadorF = INICIO_F;                          // El jugador vuelve al centro del salón
    jugadorC = INICIO_C;
    for (int i = 0; i < mesas.tamano(); i++) mesas[i].liberar();     // Todas las mesas libres
    Cliente c;
    while (filaComun.desencolar(c)) {}            // Filas vacías
    while (filaVIP.desencolar(c)) {}
    vipSeguidos = 0;
    pedidos.vaciar();                             // Sin pedidos pendientes
    cocinaCliente = -1;
    manoCliente = -1;
    manoMesa = -1;
    meseroCliente = -1;
    meseroMesa = -1;
    meseroLibreEn = 0;
    cuentas.limpiar();
    domicilios.limpiar();
    largoUltimaRuta = 0;
    hoy = ResumenJornada();                       // Estadísticas de la jornada en cero
    hoy.jornada = jornada;
    hoy.repInicio = reputacion;
    proximaLlegada = azar(1, 4);                  // El primer grupo llega enseguida
    decirJonatan(FRASES_JORNADA[jornada - 1]);
    anotar(string("Jornada ") + to_string(jornada) + ": " + JORNADAS[jornada - 1].nombre + ". ¡Abre el restaurante!");
}

void Juego::avanzarTick() {                       // TODO lo que pasa en un tick, en orden
    if (faseActual == FASE_NOCHE) {               // De noche: avanza el reloj de la noche (la moto va por una calle)
        tickNoche++;
        for (int i = 0; i < domicilios.tamano(); i++) {          // Los pedidos que se pasan de su límite se enfrían
            if (domicilios[i].estado == DOM_PENDIENTE && tickNoche == domicilios[i].limite + 1)
                anotar("El pedido " + to_string(domicilios[i].id) + " se enfrió: si llega ahora, llega tarde.");
        }
        return;
    }
    if (faseActual != FASE_DIA) return;
    tick++;                                       // Avanza el reloj
    const Jornada& J = JORNADAS[jornada - 1];

    if (amenaza >= AMENAZA_MAXIMA) atacar();      // Si la barra se llenó de noche, Jonatan ataca al empezar el día

    // 1. Llegadas: un grupo cada J.llegada ticks (±2), salvo en la última llamada
    if (tick >= proximaLlegada && tick < TICKS_DIA - ULTIMA_LLAMADA) {
        llegaGrupo();
        proximaLlegada = tick + azar(J.llegada - 2, J.llegada + 2);
    }

    // 2. Paciencia en la fila
    revisarFila();
    if (faseActual != FASE_DIA) return;           // Pudo perder por reputación

    // 3. El mesero (mejora) llega a la mesa con el plato
    if (meseroCliente != -1 && tick >= meseroLlegaEn) {
        int pos = mesaDeCliente(meseroCliente);
        if (pos != -1) {                          // El grupo todavía está
            mesas[pos].comiendoHasta = tick + TICKS_COMIENDO;
            anotar("El mesero sirvió la mesa " + to_string(mesas[pos].id) + ".");
        }
        meseroCliente = -1;                       // Ya entregó (vuelve caminando hasta meseroLibreEn)
        meseroMesa = -1;
    }

    // 4. Las mesas: terminan de comer, se cansan o llegan a la mitad de su paciencia
    for (int i = 0; i < mesas.tamano(); i++) {
        Mesa& m = mesas[i];
        if (!m.ocupada) continue;
        if (m.comiendoHasta >= 0) {               // Está comiendo
            if (tick >= m.comiendoHasta) {        // Terminó: deja su cuenta y libera la mesa
                cuentas.agregar(Cuenta(m.id, m.cuenta));
                hoy.atendidos++;
                anotar("La mesa " + to_string(m.id) + " terminó y dejó su cuenta de " + pesos(m.cuenta) + ".");
                int premio = (m.tarjeta == PRESTIGIO) ? 2 : 1;      // Prestigio da +2
                m.liberar();
                cambiarReputacion(premio);
            }
        } else if (tick >= m.tickLimite) {        // Se le acabó la paciencia sin comer
            anotar("La mesa " + to_string(m.id) + " se cansó de esperar y se fue sin pagar.");
            hoy.perdidos++;
            if (cocinaCliente == m.idCliente) cocinaCliente = -1;   // Su plato en la cocina se bota
            if (manoCliente == m.idCliente) { manoCliente = -1; manoMesa = -1; anotar("El plato que llevabas ya no tiene dueño."); }
            if (meseroCliente == m.idCliente) { meseroCliente = -1; meseroMesa = -1; }
            m.liberar();                          // Su pedido queda en el montículo y se descarta al salir
            decirJonatan(FRASES_MESA[azar(0, 2)]);
            cambiarReputacion(-CASTIGO_MESA);
            if (faseActual != FASE_DIA) return;
            subirAmenaza(AMENAZA_MESA);
        } else if (!m.avisoMitad && !m.cocinando && tick >= (m.tickSentado + m.tickLimite) / 2) {
            m.avisoMitad = true;                  // A la mitad de su paciencia y su pedido ni ha empezado
            subirAmenaza(AMENAZA_MITAD);
        }
    }

    // 5. El mesero (mejora) recoge un plato listo si está libre
    if (mejoras[MEJORA_MESERO] && meseroCliente == -1 && tick >= meseroLibreEn && cocinaCliente != -1 && tick >= cocinaListaEn) {
        int pos = mesaDeCliente(cocinaCliente);
        if (pos != -1) {
            int pasos = abs(mesas[pos].fila - (COCINA_F + 1)) + abs(mesas[pos].col - COCINA_C);    // Distancia de la cocina a la mesa
            meseroCliente = cocinaCliente;
            meseroMesa = mesas[pos].id;
            meseroLlegaEn = tick + pasos + 1;     // Llega a la mesa...
            meseroLibreEn = tick + 2 * pasos + 2; // ...y vuelve a la cocina
        }
        cocinaCliente = -1;                       // La cocina queda libre
    }

    // 6. Los obstáculos del ataque Caos se desgastan
    mapa.actualizarObstaculos();

    // 7. Fin del día
    if (tick >= TICKS_DIA) terminarDia();
}

void Juego::llegaGrupo() {
    if (filaComun.longitud() + filaVIP.longitud() >= FILA_MAXIMA) {      // Fila llena: siguen de largo, sin castigo
        anotar("Un grupo vio la fila llena y siguió de largo.");
        return;
    }
    const Jornada& J = JORNADAS[jornada - 1];
    int tarjeta = SIN_TARJETA;
    if (azar(1, 100) <= J.vip) {                  // ¿Es VIP?
        int r = azar(1, 100);                     // De los VIP: 50 % frecuente, 30 % amigo, 20 % prestigio
        tarjeta = (r <= 50) ? CLIENTE_FRECUENTE : (r <= 80 ? AMIGO_HOST : PRESTIGIO);
    }
    const int TAMANOS[8] = {1, 2, 2, 2, 3, 3, 4, 4};                     // Grupos de 1 a 4 (más seguido de 2)
    int personas = TAMANOS[azar(0, 7)];
    string nombre = string("Los ") + APELLIDOS[siguienteTurno % 12];
    Cliente c(nombre, personas, tarjeta, siguienteTurno);
    siguienteTurno++;
    c.llegada = tick;                             // Desde ya corre su paciencia en la fila
    if (tarjeta == SIN_TARJETA) filaComun.encolar(c);                    // Común → cola normal
    else filaVIP.encolar(c);                                             // VIP → cola con prioridad
    anotar("Llegó " + nombre + " (" + to_string(personas) + ", " + NOMBRE_TIPO[tarjeta] + ").");
}

void Juego::seFueDeLaFila(const Cliente& c) {
    anotar(c.nombre + " se cansó de esperar en la fila y se fue.");
    decirJonatan(FRASES_FILA[azar(0, 2)]);
    hoy.perdidos++;
    cambiarReputacion(-CASTIGO_FILA);
    if (faseActual == FASE_DIA) subirAmenaza(AMENAZA_FILA);
}

void Juego::revisarFila() {
    // Se saca a todos y se vuelven a meter los que siguen esperando, usando una cola temporal.
    // (Con la cola con prioridad no sirve sacar y volver a meter en la misma: el nivel más alto nunca se vaciaría.)
    Cliente c;
    Cola tempComun;
    while (filaComun.desencolar(c)) {
        if (tick >= c.llegada + PACIENCIA_FILA) seFueDeLaFila(c);
        else tempComun.encolar(c);
    }
    while (tempComun.desencolar(c)) filaComun.encolar(c);              // Vuelven en el mismo orden
    ColaPrioridad tempVIP;
    while (filaVIP.desencolar(c)) {
        if (tick >= c.llegada + PACIENCIA_FILA) seFueDeLaFila(c);
        else tempVIP.encolar(c);
    }
    while (tempVIP.desencolar(c)) filaVIP.encolar(c);                  // Cada nivel conserva su orden de llegada
}

bool Juego::siguienteCliente(Cliente& c, bool& deVip) {          // Regla de la fase 2 (Estado.h): 2 VIP seguidos y luego un común
    if (vipSeguidos >= VIP_POR_COMUN && filaComun.frente(c)) { deVip = false; return true; }
    if (filaVIP.frente(c)) { deVip = true; return true; }
    if (filaComun.frente(c)) { deVip = false; return true; }
    return false;                                 // Ambas filas vacías
}

Resultado Juego::sentar(int idMesa) {
    if (faseActual != FASE_DIA) return Resultado(false, "Ahora no se puede sentar a nadie.");
    Cliente c;
    bool deVip = false;
    if (!siguienteCliente(c, deVip)) return Resultado(false, "No hay nadie esperando en la fila.");
    int pos = buscarMesa(idMesa);
    if (pos == -1) return Resultado(false, "Esa mesa no existe.");
    if (mesas[pos].ocupada) return Resultado(false, "La mesa " + to_string(idMesa) + " está ocupada.");
    if (mesas[pos].capacidad < c.personas)
        return Resultado(false, c.nombre + " son " + to_string(c.personas) + " y en la mesa " + to_string(idMesa) + " caben " + to_string(mesas[pos].capacidad) + ".");

    Cliente sale;                                 // Sale de su fila
    if (deVip) { filaVIP.desencolar(sale); vipSeguidos++; }
    else { filaComun.desencolar(sale); vipSeguidos = 0; }

    const Jornada& J = JORNADAS[jornada - 1];
    Mesa& m = mesas[pos];
    int plato = azar(0, menu.tamano() - 1);       // El grupo pide un producto del menú (combo para todos)
    int paciencia = (J.paciencia + (mejoras[MEJORA_AMBIENTE] ? AMBIENTE_EXTRA : 0)) * PACIENCIA_TIPO[sale.tarjeta] / 100;
    m.ocupada = true;
    m.idCliente = sale.turno;
    m.personas = sale.personas;
    m.tarjeta = sale.tarjeta;
    m.plato = plato;
    m.tickSentado = tick;
    m.tickLimite = tick + paciencia;
    m.comiendoHasta = -1;
    m.cuenta = menu[plato].precio * sale.personas * PAGO_TIPO[sale.tarjeta] / 100;
    m.cocinando = false;
    m.avisoMitad = false;
    pedidos.insertar(ElementoMonticulo(sale.turno, -m.tickLimite));    // Clave negativa: el que se va primero queda en la raíz
    string msg = sale.nombre + " se sientan en la mesa " + to_string(idMesa) + " y piden " + menu[plato].nombre + ".";
    anotar(msg);
    avanzarTick();
    return Resultado(true, msg);
}

Resultado Juego::mover(char direccion) {
    if (faseActual != FASE_DIA) return Resultado(false, "Ahora no puedes caminar.");
    int f = jugadorF, c = jugadorC;
    if (direccion == 'A') f--;                    // Arriba
    else if (direccion == 'B') f++;               // Abajo
    else if (direccion == 'I') c--;               // Izquierda
    else if (direccion == 'D') c++;               // Derecha
    else return Resultado(false, "Dirección no válida.");
    if (!mapa.esTransitable(f, c)) return Resultado(false, "No puedes pasar por ahí.");     // No gasta tick
    jugadorF = f;
    jugadorC = c;
    avanzarTick();
    return Resultado(true, "");
}

Resultado Juego::esperar() {
    if (faseActual != FASE_DIA) return Resultado(false, "Ahora no se puede esperar.");
    avanzarTick();
    return Resultado(true, "Esperas un momento.");
}

Resultado Juego::interactuar() {                  // Tecla E
    if (faseActual != FASE_DIA) return Resultado(false, "Ahora no hay nada que usar.");

    // 1. Si lleva un plato y está junto a su mesa: lo sirve
    if (manoCliente != -1) {
        int pos = mesaDeCliente(manoCliente);
        if (pos != -1 && vecinoDe(mesas[pos].fila, mesas[pos].col)) {
            mesas[pos].comiendoHasta = tick + TICKS_COMIENDO;
            string msg = "Sirves la mesa " + to_string(mesas[pos].id) + ". ¡A comer!";
            manoCliente = -1;
            manoMesa = -1;
            anotar(msg);
            avanzarTick();
            return Resultado(true, msg);
        }
    }

    // 2. La cocina: recoger el plato listo y empezar el pedido más urgente
    if (vecinoDe(COCINA_F, COCINA_C)) {
        string msg;
        bool hizoAlgo = false;
        if (cocinaCliente != -1 && tick >= cocinaListaEn) {          // Hay un plato listo
            if (manoCliente != -1) return Resultado(false, "Ya llevas un plato. Entrégalo primero.");
            int pos = mesaDeCliente(cocinaCliente);
            manoCliente = cocinaCliente;
            manoMesa = mesas[pos].id;
            cocinaCliente = -1;
            msg = "Recoges " + menu[mesas[pos].plato].nombre + " para la mesa " + to_string(manoMesa) + ".";
            hizoAlgo = true;
        }
        if (cocinaCliente == -1) {                // La cocina está libre: empieza el pedido de la raíz
            ElementoMonticulo e;
            while (pedidos.extraer(e)) {          // Sale el más urgente
                int pos = mesaDeCliente(e.dato);
                if (pos == -1 || mesas[pos].cocinando) continue;     // Ese grupo ya se fue: el pedido se descarta
                Mesa& m = mesas[pos];
                int prep = menu[m.plato].ticksPreparacion - (mejoras[MEJORA_COCINA] ? 1 : 0);
                if (prep < 1) prep = 1;
                cocinaCliente = e.dato;
                cocinaListaEn = tick + prep;
                m.cocinando = true;
                msg += (msg.empty() ? "" : " ") + string("La cocina prepara ") + menu[m.plato].nombre + " para la mesa " + to_string(m.id) + ".";
                hizoAlgo = true;
                break;
            }
        }
        if (!hizoAlgo) {                          // Nada que hacer: no gasta tick
            if (cocinaCliente != -1) return Resultado(false, "La cocina sigue preparando: faltan " + to_string(cocinaListaEn - tick) + " tick(s). Espera con la barra espaciadora.");
            return Resultado(false, "No hay pedidos esperando.");
        }
        anotar(msg);
        avanzarTick();
        return Resultado(true, msg);
    }

    // 3. La caja: cobra todas las cuentas pendientes
    if (vecinoDe(CAJA_F, CAJA_C)) {
        if (cuentas.estaVacio()) return Resultado(false, "No hay cuentas por cobrar.");
        int total = 0;
        for (int i = 0; i < cuentas.tamano(); i++) {                 // Pasa cada cuenta al registro de ventas
            total += cuentas[i].monto;
            ventas.agregar(Venta(jornada, "Mesa " + to_string(cuentas[i].mesa), cuentas[i].monto));
        }
        int n = cuentas.tamano();
        cuentas.limpiar();
        dinero += total;
        hoy.ventasDia += total;
        string msg = "Cobras " + to_string(n) + " cuenta(s): " + pesos(total) + ".";
        anotar(msg);
        avanzarTick();
        return Resultado(true, msg);
    }

    // 4. Una mesa sin plato para ella
    if (manoCliente != -1) return Resultado(false, "Ese plato es para la mesa " + to_string(manoMesa) + ".");
    int pos = mesaVecina();
    if (pos != -1) {
        const Mesa& m = mesas[pos];
        if (!m.ocupada) return Resultado(false, "La mesa " + to_string(m.id) + " está libre.");
        if (m.comiendoHasta >= 0) return Resultado(false, "La mesa " + to_string(m.id) + " está comiendo.");
        return Resultado(false, "La mesa " + to_string(m.id) + " espera " + menu[m.plato].nombre + ". Ve a la cocina.");
    }
    return Resultado(false, "No hay nada para usar aquí. Acércate a la cocina, a la caja o a una mesa.");
}

Resultado Juego::cerrarTemprano() {
    if (faseActual != FASE_DIA) return Resultado(false, "Ahora no se puede cerrar.");
    if (tick < TICKS_DIA - ULTIMA_LLAMADA) return Resultado(false, "Todavía pueden llegar clientes.");
    for (int i = 0; i < mesas.tamano(); i++) if (mesas[i].ocupada) return Resultado(false, "Todavía hay mesas ocupadas.");
    if (!filaComun.esvacia() || !filaVIP.esvacia()) return Resultado(false, "Todavía hay gente en la fila.");
    if (!cuentas.estaVacio()) return Resultado(false, "Cobra las cuentas antes de cerrar.");
    anotar("Cierras temprano: " + to_string(TICKS_DIA - tick) + " tick(s) pasan a la noche.");
    terminarDia();
    return Resultado(true, "Cierras el restaurante. ¡A repartir domicilios!");
}

void Juego::terminarDia() {
    int sinCobrar = 0;                            // Las cuentas que no se cobraron se pierden
    for (int i = 0; i < cuentas.tamano(); i++) sinCobrar += cuentas[i].monto;
    for (int i = 0; i < mesas.tamano(); i++) {
        Mesa& m = mesas[i];
        if (!m.ocupada) continue;
        if (m.comiendoHasta >= 0) sinCobrar += m.cuenta;           // Estaban comiendo: se van sin pasar por la caja
        else {                                    // No alcanzaron a comer
            hoy.perdidos++;
            cambiarReputacion(-CASTIGO_MESA);
        }
        m.liberar();
    }
    if (sinCobrar > 0) anotar("Se perdieron " + pesos(sinCobrar) + " en cuentas sin cobrar.");
    hoy.perdidoSinCobrar = sinCobrar;
    hoy.repFinDia = reputacion;
    cuentas.limpiar();
    Cliente c;
    while (filaComun.desencolar(c)) {}            // La fila se va a casa (ya era tarde)
    while (filaVIP.desencolar(c)) {}
    if (faseActual == FASE_FINAL) return;         // Perdió por reputación

    // ---------- Prepara la noche ----------
    int sobrantes = TICKS_DIA - tick;             // Ticks que sobraron si cerró temprano
    if (sobrantes < 0) sobrantes = 0;
    ciudad.sortearCostos();                       // Costos de las calles de esta jornada (1 a 5)
    if (jornada == 3) ciudad.escalarCostos(150);  // Noche lluviosa: +50 %
    for (int k = 0; k < traficoPendiente; k++) ciudad.traficoInfernal(CALLES_TRAFICO, AUMENTO_TRAFICO);
    if (traficoPendiente > 0) anotar("Tráfico: hay calles más lentas esta noche.");
    traficoPendiente = 0;
    int casas[6] = {5, 6, 7, 8, 9, 10};           // Las casas se sortean sin repetir
    for (int i = 5; i > 0; i--) {
        int j = azar(0, i);
        int aux = casas[i]; casas[i] = casas[j]; casas[j] = aux;
    }
    domicilios.limpiar();
    for (int i = 0; i < JORNADAS[jornada - 1].domicilios; i++) {
        domicilios.agregar(Domicilio(i + 1, casas[i % 6], azar(LIMITE_DOM_MIN, LIMITE_DOM_MAX)));
    }
    tickNoche = 0;
    duracionNoche = TICKS_NOCHE + sobrantes;
    largoUltimaRuta = 0;
    posMoto = RESTAURANTE;                        // La moto espera en el restaurante
    estadoNoche = NOCHE_EN_BASE;
    numCarga = 0;
    largoPlan = 0;
    idxPlan = 0;
    gpsVisto = false;
    largoRecorrido = 0;
    numEntregasRecientes = 0;
    calleCerradaAhora = -1;
    cierresRestantes = JORNADAS[jornada - 1].cierres;           // Calles que Jonatan puede cerrar esta noche
    proximoCierre = azar(CIERRE_PRIMERO_MIN, CIERRE_PRIMERO_MAX);
    urgenteOfrecido = false;
    hayOfertaUrgente = false;
    hayUltimoViaje = false;
    faseActual = FASE_NOCHE;
    decirJonatan(FRASES_NOCHE[jornada - 1]);
    anotar("Llega la noche: " + to_string(domicilios.tamano()) + " domicilios y " + to_string(duracionNoche) + " ticks.");
}


// ============================================================
// La noche: el jugador traza su ruta, el GPS (Dijkstra) la compara y Jonatan puede cerrar calles
// ============================================================

int Juego::costoCalle(int a, int b) {             // Costo de la calle abierta entre a y b (-1 si no hay o está cerrada)
    int calle = ciudad.calleEntre(a, b);
    if (calle == -1 || ciudad.estaCerrada(calle)) return -1;
    return ciudad.costoCalle(calle);
}

int Juego::costoDeRuta(const int ruta[], int largo) {             // Suma de las calles de la ruta (-1 si alguna no existe o está cerrada)
    int total = 0;
    for (int i = 0; i + 1 < largo; i++) {
        int c = costoCalle(ruta[i], ruta[i + 1]);
        if (c < 0) return -1;
        total += c;
    }
    return total;
}

bool Juego::validarRuta(const int ruta[], int largo, int desde, string& error) {
    if (largo < 2) { error = "Traza la ruta en el mapa: haz clic en las esquinas y las casas, una tras otra."; return false; }
    if (largo > MAX_RUTA) { error = "La ruta es demasiado larga (máximo " + to_string(MAX_RUTA) + " lugares)."; return false; }
    for (int i = 0; i < largo; i++) {
        if (ruta[i] < 0 || ruta[i] >= ciudad.cantidadNodos()) { error = "La ruta tiene un lugar que no existe."; return false; }
    }
    if (ruta[0] != desde) { error = "La ruta debe empezar donde está la moto: " + ciudad.nombreNodo(desde) + "."; return false; }
    if (ruta[largo - 1] != RESTAURANTE) { error = "La ruta debe terminar en el restaurante."; return false; }
    for (int i = 0; i + 1 < largo; i++) {
        int calle = ciudad.calleEntre(ruta[i], ruta[i + 1]);
        if (calle == -1) { error = "No hay calle entre " + ciudad.nombreNodo(ruta[i]) + " y " + ciudad.nombreNodo(ruta[i + 1]) + "."; return false; }
        if (ciudad.estaCerrada(calle)) { error = "La calle entre " + ciudad.nombreNodo(ruta[i]) + " y " + ciudad.nombreNodo(ruta[i + 1]) + " está cerrada."; return false; }
    }
    return true;
}

bool Juego::rutaVisita(const int ruta[], int largo, int casa) const {
    for (int i = 0; i < largo; i++) if (ruta[i] == casa) return true;
    return false;
}

int Juego::optimoDesde(int desde, const int casas[], int n) {      // Ruta más rápida desde "desde" por las casas (en el mejor orden) y de vuelta
    ciudad.calcularRutas(desde);                  // Dijkstra desde donde está la moto
    int d0 = ciudad.distanciaA(RESTAURANTE);
    if (n == 0) return d0;
    int dA = ciudad.distanciaA(casas[0]);
    int dB = (n == 2) ? ciudad.distanciaA(casas[1]) : 0;
    ciudad.calcularRutas(casas[0]);               // Dijkstra desde la primera casa
    int dA0 = ciudad.distanciaA(RESTAURANTE);
    if (n == 1) return (dA >= INFINITO || dA0 >= INFINITO) ? INFINITO : dA + dA0;
    int dAB = ciudad.distanciaA(casas[1]);
    ciudad.calcularRutas(casas[1]);               // Dijkstra desde la segunda casa
    int dB0 = ciudad.distanciaA(RESTAURANTE);
    if (dA >= INFINITO || dB >= INFINITO || dAB >= INFINITO || dA0 >= INFINITO || dB0 >= INFINITO) return INFINITO;
    int primeroA = dA + dAB + dB0;                // desde → A → B → restaurante
    int primeroB = dB + dAB + dA0;                // desde → B → A → restaurante
    return primeroA <= primeroB ? primeroA : primeroB;
}

bool Juego::armarRutaOptima(int desde, const int casas[], int n, int ruta[], int& largo) {
    int orden[2] = {n > 0 ? casas[0] : -1, n > 1 ? casas[1] : -1};
    if (n == 2) {                                 // Con dos casas se escoge el orden: primero el más rápido;
        ciudad.calcularRutas(desde);              // si tardan lo mismo, el que deja más pedidos a tiempo
        int dA = ciudad.distanciaA(casas[0]), dB = ciudad.distanciaA(casas[1]);
        ciudad.calcularRutas(casas[0]);
        int dA0 = ciudad.distanciaA(RESTAURANTE), dAB = ciudad.distanciaA(casas[1]);
        ciudad.calcularRutas(casas[1]);
        int dB0 = ciudad.distanciaA(RESTAURANTE);
        int totalA = dA + dAB + dB0, totalB = dB + dAB + dA0;
        int limA = limiteDeCasa(casas[0]), limB = limiteDeCasa(casas[1]);
        int tardeA = (tickNoche + dA > limA) + (tickNoche + dA + dAB > limB);     // Pedidos tarde si va primero a A
        int tardeB = (tickNoche + dB > limB) + (tickNoche + dB + dAB > limA);     // Pedidos tarde si va primero a B
        bool primeroB = totalB < totalA || (totalB == totalA && (tardeB < tardeA || (tardeB == tardeA && limB < limA)));
        if (primeroB) { orden[0] = casas[1]; orden[1] = casas[0]; }
    }
    int paradas[4];                               // desde → casa(s) → restaurante
    int np = 0;
    paradas[np++] = desde;
    for (int i = 0; i < n; i++) paradas[np++] = orden[i];
    paradas[np++] = RESTAURANTE;
    largo = 0;
    for (int i = 0; i + 1 < np; i++) {            // Un tramo por cada par de paradas seguidas
        ciudad.calcularRutas(paradas[i]);         // Dijkstra desde el inicio del tramo
        if (ciudad.distanciaA(paradas[i + 1]) >= INFINITO) { largo = 0; return false; }
        int tramo[MAX_NODOS];
        int lt = 0;
        ciudad.rutaA(paradas[i + 1], tramo, lt);  // Nodos del tramo
        for (int k = (i == 0 ? 0 : 1); k < lt; k++) {                // Sin repetir el nodo de unión
            if (largo >= MAX_RUTA) { largo = 0; return false; }
            ruta[largo++] = tramo[k];
        }
    }
    return largo >= 2;
}

int Juego::limiteDeCasa(int casa) const {        // Tiempo límite del pedido pendiente de esa casa (cada casa tiene máximo un pedido por noche)
    for (int i = 0; i < domicilios.tamano(); i++) if (domicilios[i].casa == casa && domicilios[i].estado == DOM_PENDIENTE) return domicilios[i].limite;
    return INFINITO;
}

int Juego::casasPendientesEnMoto(int casas[]) {   // Casas de los pedidos que lleva la moto y todavía no entrega
    int n = 0;
    for (int k = 0; k < numCarga; k++) {
        const Domicilio& d = domicilios[carga[k] - 1];
        if (d.estado == DOM_PENDIENTE) casas[n++] = d.casa;
    }
    return n;
}

bool Juego::quedanPendientes() const {
    for (int i = 0; i < domicilios.tamano(); i++) if (domicilios[i].estado == DOM_PENDIENTE) return true;
    return false;
}

bool Juego::rutaOptima(int idA, int idB, int ruta[], int& largo) {    // Para el GPS y el jugador automático
    largo = 0;
    if (faseActual != FASE_NOCHE) return false;
    int casas[2];
    int n = 0;
    if (estadoNoche == NOCHE_DESVIO) n = casasPendientesEnMoto(casas);
    else {
        if (idA < 1 || idA > domicilios.tamano() || domicilios[idA - 1].estado != DOM_PENDIENTE) return false;
        casas[n++] = domicilios[idA - 1].casa;
        if (idB != -1) {
            if (idB < 1 || idB > domicilios.tamano() || idB == idA || domicilios[idB - 1].estado != DOM_PENDIENTE) return false;
            casas[n++] = domicilios[idB - 1].casa;
        }
    }
    return armarRutaOptima(posMoto, casas, n, ruta, largo);
}

int Juego::duracionViaje(int idA, int idB) {      // Ticks de la ruta más rápida desde el restaurante (-1 si no se puede)
    if (faseActual != FASE_NOCHE || idA < 1 || idA > domicilios.tamano()) return -1;
    int casas[2] = {domicilios[idA - 1].casa, -1};
    int n = 1;
    if (idB != -1) {
        if (idB < 1 || idB > domicilios.tamano() || idB == idA) return -1;
        casas[n++] = domicilios[idB - 1].casa;
    }
    int d = optimoDesde(RESTAURANTE, casas, n);
    return d >= INFINITO ? -1 : d;
}

Resultado Juego::verRuta(int idA, int idB) {      // El GPS muestra la ruta más rápida (y el siguiente viaje ya no da bono)
    if (faseActual != FASE_NOCHE) return Resultado(false, "El GPS es para la noche.");
    if (hayOfertaUrgente) return Resultado(false, "Primero responde el pedido urgente.");
    if (estadoNoche != NOCHE_DESVIO && idB != -1 && !mejoras[MEJORA_MOTO]) return Resultado(false, "Para llevar 2 pedidos necesitas la moto mejorada.");
    int r[MAX_RUTA];
    int largo = 0;
    if (!rutaOptima(idA, idB, r, largo)) return Resultado(false, estadoNoche == NOCHE_DESVIO ? "El GPS no encontró camino." : "Elige primero uno o dos pedidos pendientes.");
    for (int i = 0; i < largo; i++) ultimaRuta[i] = r[i];
    largoUltimaRuta = largo;
    gpsVisto = true;
    string msg = "GPS: ";
    for (int i = 0; i < largo; i++) msg += (i ? " → " : "") + ciudad.nombreNodo(r[i]);
    msg += " (" + to_string(costoDeRuta(r, largo)) + " ticks). Este viaje ya no da bono de ruta perfecta.";
    return Resultado(true, msg);
}

Resultado Juego::salir(int idA, int idB, const int ruta[], int largo) {
    if (faseActual != FASE_NOCHE) return Resultado(false, "Los domicilios son de noche.");
    if (estadoNoche == NOCHE_DESVIO) return Resultado(false, "La moto está en la calle: traza el desvío.");
    if (hayOfertaUrgente) return Resultado(false, "Primero responde el pedido urgente.");
    if (idB != -1 && !mejoras[MEJORA_MOTO]) return Resultado(false, "Para llevar 2 pedidos necesitas la moto mejorada.");
    if (idA < 1 || idA > domicilios.tamano() || domicilios[idA - 1].estado != DOM_PENDIENTE) return Resultado(false, "Elige un pedido pendiente.");
    if (idB != -1 && (idB < 1 || idB > domicilios.tamano() || idB == idA || domicilios[idB - 1].estado != DOM_PENDIENTE))
        return Resultado(false, "El segundo pedido no está pendiente.");
    string error;
    if (!validarRuta(ruta, largo, RESTAURANTE, error)) return Resultado(false, error);
    int casas[2];
    int n = 0;
    int ids[2] = {idA, idB};
    for (int k = 0; k < 2; k++) {
        if (ids[k] == -1) continue;
        const Domicilio& d = domicilios[ids[k] - 1];
        if (!rutaVisita(ruta, largo, d.casa)) return Resultado(false, "Tu ruta no pasa por " + ciudad.nombreNodo(d.casa) + " (pedido " + to_string(d.id) + ").");
        casas[n++] = d.casa;
    }
    int costo = costoDeRuta(ruta, largo);
    if (tickNoche + costo > duracionNoche)
        return Resultado(false, "No alcanzas a volver: tu ruta toma " + to_string(costo) + " ticks y quedan " + to_string(duracionNoche - tickNoche) + ".");

    for (int i = 0; i < largo; i++) plan[i] = ruta[i];   // La moto sale con esa ruta
    largoPlan = largo;
    idxPlan = 0;
    carga[0] = idA;
    carga[1] = idB;
    numCarga = idB == -1 ? 1 : 2;
    pasosViaje = 0;
    costoViaje = 0;
    optimoViaje = optimoDesde(RESTAURANTE, casas, n);    // Lo que tardaría la ruta del GPS (Dijkstra)
    viajePerfecto = (costo == optimoViaje);
    huboCierreEnViaje = false;
    hayUltimoViaje = false;
    largoUltimaRuta = 0;
    mensajeViaje = "";
    anotar("Sale la moto con " + string(numCarga == 2 ? "los pedidos " + to_string(idA) + " y " + to_string(idB) : "el pedido " + to_string(idA)) +
           ": tu ruta tarda " + to_string(costo) + " ticks.");
    viajar();
    return Resultado(true, mensajeViaje);
}

Resultado Juego::desviar(const int ruta[], int largo) {
    if (!enDesvio()) return Resultado(false, "La moto no está detenida.");
    string error;
    if (!validarRuta(ruta, largo, posMoto, error)) return Resultado(false, error);
    int casas[2];
    int n = casasPendientesEnMoto(casas);
    for (int k = 0; k < n; k++) {
        if (!rutaVisita(ruta, largo, casas[k])) return Resultado(false, "Tu desvío no pasa por " + ciudad.nombreNodo(casas[k]) + ".");
    }
    int costo = costoDeRuta(ruta, largo);
    int optimo = optimoDesde(posMoto, casas, n);  // El GPS recalcula desde la esquina donde quedó la moto
    if (costo != optimo) viajePerfecto = false;
    for (int i = 0; i < largo; i++) plan[i] = ruta[i];
    largoPlan = largo;
    idxPlan = 0;
    estadoNoche = NOCHE_EN_BASE;                  // Sigue el viaje
    largoUltimaRuta = 0;
    mensajeViaje = "";
    anotar("Desvío desde " + ciudad.nombreNodo(posMoto) + ": " + to_string(costo) + " ticks más (el GPS decía " + to_string(optimo) + ").");
    viajar();
    return Resultado(true, mensajeViaje);
}

void Juego::viajar() {                            // La moto recorre su ruta calle por calle
    largoRecorrido = 0;
    recorrido[largoRecorrido++] = posMoto;
    numEntregasRecientes = 0;
    calleCerradaAhora = -1;
    int pendientes[2];
    while (idxPlan + 1 < largoPlan) {
        int sig = plan[idxPlan + 1];
        int calle = ciudad.calleEntre(posMoto, sig);
        // ¿Jonatan cierra justo la calle que sigue? (solo a mitad de camino y sin dejar ningún lugar aislado)
        if (cierresRestantes > 0 && tickNoche >= proximoCierre && pasosViaje >= 1 && posMoto != RESTAURANTE && ciudad.sePuedeCerrar(calle)) {
            ciudad.cerrarCalle(calle);
            cierresRestantes--;
            proximoCierre = tickNoche + azar(CIERRE_SEPARACION_MIN, CIERRE_SEPARACION_MAX);
            calleCerradaAhora = calle;
            huboCierreEnViaje = true;
            hoy.cierres++;
            estadoNoche = NOCHE_DESVIO;
            decirJonatan(FRASES_CIERRE[azar(0, 2)]);
            string txt = "¡Jonatan cerró la calle entre " + ciudad.nombreNodo(posMoto) + " y " + ciudad.nombreNodo(sig) + "! Traza un desvío.";
            anotar(txt);
            mensajeViaje += txt;
            return;
        }
        int costo = ciudad.costoCalle(calle);
        for (int k = 0; k < costo; k++) avanzarTick();   // Cada tick de la calle pasa por avanzarTick()
        costoViaje += costo;
        posMoto = sig;
        idxPlan++;
        pasosViaje++;
        if (largoRecorrido < MAX_RUTA * 2) recorrido[largoRecorrido++] = posMoto;
        llegarA(posMoto);
        if (faseActual == FASE_FINAL) return;     // Perdió por reputación
        if (posMoto == RESTAURANTE && casasPendientesEnMoto(pendientes) == 0) break;    // Ya entregó todo y está en casa
    }
    terminarViaje();
}

void Juego::llegarA(int nodo) {                   // Entrega los pedidos de esa casa (si la moto los lleva)
    for (int k = 0; k < numCarga; k++) {
        Domicilio& d = domicilios[carga[k] - 1];
        if (d.estado != DOM_PENDIENTE || d.casa != nodo) continue;
        d.llegada = tickNoche;
        int pago;
        string txt;
        if (d.llegada <= d.limite) {              // A tiempo: base + extra por cada tick que sobre
            pago = d.urgente ? PAGO_URGENTE + PAGO_URGENTE_TICK * (d.limite - d.llegada)
                             : PAGO_A_TIEMPO + PAGO_POR_TICK * (d.limite - d.llegada);
            d.estado = DOM_A_TIEMPO;
            hoy.aTiempo++;
            txt = "Pedido " + to_string(d.id) + " llegó caliente: " + pesos(pago) + ". ";
        } else {                                  // Tarde: la mitad y baja la reputación
            pago = d.urgente ? PAGO_URGENTE_TARDE : PAGO_TARDE;
            d.estado = DOM_TARDE;
            txt = "Pedido " + to_string(d.id) + " llegó frío: " + pesos(pago) + ". ";
            decirJonatan("Llegó frío. Como su servicio.");
        }
        d.pago = pago;
        dinero += pago;
        hoy.ganadoNoche += pago;
        hoy.entregas++;
        ventas.agregar(Venta(jornada, string(d.urgente ? "Urgente a " : "Domicilio a ") + ciudad.nombreNodo(d.casa), pago));
        if (numEntregasRecientes < 4) {           // Para que la página anime la entrega en el lugar correcto
            entregaId[numEntregasRecientes] = d.id;
            entregaPaso[numEntregasRecientes] = largoRecorrido - 1;
            numEntregasRecientes++;
        }
        anotar(txt);
        mensajeViaje += txt;
        if (d.estado == DOM_TARDE) {
            cambiarReputacion(-CASTIGO_TARDE);
            if (faseActual == FASE_FINAL) return;
            subirAmenaza(AMENAZA_TARDE);
        }
    }
}

void Juego::terminarViaje() {                     // La moto volvió al restaurante
    ultimoCosto = costoViaje;
    ultimoOptimo = optimoViaje;
    ultimoConCierre = huboCierreEnViaje;
    ultimoPerfecto = viajePerfecto && !gpsVisto;
    ultimoBono = 0;
    if (ultimoPerfecto) {                         // Igualó al GPS sin mirarlo
        ultimoBono = BONO_RUTA_PERFECTA;
        dinero += ultimoBono;
        hoy.ganadoNoche += ultimoBono;
        hoy.rutasPerfectas++;
        ventas.agregar(Venta(jornada, "Bono ruta perfecta", ultimoBono));
        mensajeViaje += "¡Ruta perfecta! +" + pesos(ultimoBono) + ".";
        anotar("¡Ruta perfecta! Fue tan rápida como la del GPS: +" + pesos(ultimoBono) + ".");
        decirJonatan(FRASES_RUTA[azar(0, 2)]);
    } else if (!gpsVisto && !huboCierreEnViaje) {
        mensajeViaje += "Tu ruta tardó " + to_string(costoViaje) + " ticks; la del GPS, " + to_string(optimoViaje) + ".";
    }
    hayUltimoViaje = true;
    hoy.viajes++;
    numCarga = 0;
    estadoNoche = NOCHE_EN_BASE;
    posMoto = RESTAURANTE;
    gpsVisto = false;
    largoPlan = 0;
    idxPlan = 0;
    if (!urgenteOfrecido && jornada >= URGENTE_DESDE) ofrecerUrgente();
    if (!hayOfertaUrgente && !quedanPendientes()) terminarJornada();     // Entregó todo: termina la noche
}

void Juego::ofrecerUrgente() {                    // Una vez por noche, al volver del primer viaje
    urgenteOfrecido = true;
    bool usada[MAX_NODOS];
    for (int i = 0; i < MAX_NODOS; i++) usada[i] = false;
    for (int i = 0; i < domicilios.tamano(); i++) usada[domicilios[i].casa] = true;
    int libres[6];
    int n = 0;
    for (int c = 5; c <= 10; c++) if (!usada[c]) libres[n++] = c;
    if (n == 0) return;                           // Todas las casas ya tienen pedido
    int casa = libres[azar(0, n - 1)];
    ciudad.calcularRutas(RESTAURANTE);
    int ida = ciudad.distanciaA(casa);
    if (ida >= INFINITO || tickNoche + 2 * ida > duracionNoche) return;     // Ya no alcanzaría: no se ofrece
    ofertaUrgente = Domicilio(domicilios.tamano() + 1, casa, tickNoche + ida + azar(HOLGURA_URGENTE_MIN, HOLGURA_URGENTE_MAX), true);
    hayOfertaUrgente = true;
    decirJonatan("¿Un pedido urgente? Veamos si llega caliente.");
    anotar("¡Pedido urgente para " + ciudad.nombreNodo(casa) + "! Paga el doble si llega antes del tick " + to_string(ofertaUrgente.limite) + ".");
}

Resultado Juego::responderUrgente(bool acepta) {
    if (faseActual != FASE_NOCHE || !hayOfertaUrgente) return Resultado(false, "No hay ningún pedido urgente.");
    hayOfertaUrgente = false;
    string msg;
    if (acepta) {
        domicilios.agregar(ofertaUrgente);        // Entra a la lista de domicilios de la noche
        hoy.urgentes++;
        msg = "Aceptaste el pedido urgente " + to_string(ofertaUrgente.id) + " para " + ciudad.nombreNodo(ofertaUrgente.casa) + ".";
    } else {
        msg = "Rechazaste el pedido urgente. No pasa nada.";
    }
    anotar(msg);
    if (!quedanPendientes()) terminarJornada();
    return Resultado(true, msg);
}

Resultado Juego::terminarNoche() {
    if (faseActual != FASE_NOCHE) return Resultado(false, "No es de noche.");
    if (estadoNoche == NOCHE_DESVIO) return Resultado(false, "La moto está en la calle: traza el desvío para volver.");
    if (hayOfertaUrgente) { hayOfertaUrgente = false; anotar("El pedido urgente se quedó sin repartidor."); }
    int faltan = 0;
    for (int i = 0; i < domicilios.tamano(); i++) if (domicilios[i].estado == DOM_PENDIENTE) faltan++;
    if (faltan > 0) {
        anotar(to_string(faltan) + " domicilio(s) sin entregar.");
        cambiarReputacion(-CASTIGO_SIN_ENTREGAR * faltan);
    }
    if (faseActual != FASE_FINAL) terminarJornada();
    return Resultado(true, faltan > 0 ? "Se acabó la noche con domicilios sin entregar." : "Se acabó la noche.");
}

void Juego::terminarJornada() {
    bool perfecta = domicilios.tamano() > 0;      // Perfecta = todos a tiempo
    for (int i = 0; i < domicilios.tamano(); i++) if (domicilios[i].estado != DOM_A_TIEMPO) perfecta = false;
    if (perfecta) { nochesPerfectas++; anotar("¡Noche perfecta! +50 puntos."); decirJonatan("Hmm. Puntuales. No me lo esperaba."); }
    else decirJonatan("Mañana vuelvo. Y voy a estar de peor humor.");
    hoy.pedidos = domicilios.tamano();
    hoy.reputacion = reputacion;
    hoy.dinero = dinero;
    historial.agregar(hoy);                       // Se guarda en el historial (arreglo dinámico)
    if (jornada == NUM_JORNADAS) { finalizar(true); return; }        // Era la última: auditoría final
    faseActual = FASE_TIENDA;
    comproEstaJornada = false;
}


// ============================================================
// Tienda de mejoras
// ============================================================

Resultado Juego::comprarMejora(int tipo) {
    if (faseActual != FASE_TIENDA) return Resultado(false, "Las mejoras se compran al terminar la jornada.");
    if (tipo < 0 || tipo >= NUM_MEJORAS) return Resultado(false, "Esa mejora no existe.");
    if (comproEstaJornada) return Resultado(false, "Solo se puede comprar una mejora por jornada.");
    if (mejoras[tipo]) return Resultado(false, string("Ya tienes ") + NOMBRE_MEJORA[tipo] + ".");
    if (dinero < PRECIO_MEJORA[tipo]) return Resultado(false, "No te alcanza: cuesta " + pesos(PRECIO_MEJORA[tipo]) + ".");
    dinero -= PRECIO_MEJORA[tipo];
    mejoras[tipo] = true;
    comproEstaJornada = true;
    ventas.agregar(Venta(jornada, string("Compra: ") + NOMBRE_MEJORA[tipo], -PRECIO_MEJORA[tipo]));
    string msg = string("Compraste ") + NOMBRE_MEJORA[tipo] + ".";
    anotar(msg);
    return Resultado(true, msg);
}

Resultado Juego::siguienteJornada() {
    if (faseActual != FASE_TIENDA) return Resultado(false, "Todavía no termina la jornada.");
    jornada++;
    iniciarDia();
    return Resultado(true, string("Jornada ") + to_string(jornada) + ": " + JORNADAS[jornada - 1].nombre + ".");
}


// ============================================================
// Jonatan, reputación y final
// ============================================================

void Juego::subirAmenaza(int cantidad) {
    amenaza += cantidad;
    if (amenaza >= AMENAZA_MAXIMA) {
        if (faseActual == FASE_DIA) atacar();     // De día ataca enseguida
        else amenaza = AMENAZA_MAXIMA;            // De noche espera al día siguiente
    }
}

void Juego::atacar() {
    amenaza = AMENAZA_TRAS_ATAQUE;
    int disponibles = JORNADAS[jornada - 1].ataques;
    if (disponibles == 0) {                       // Jornada 1: solo observa
        decirJonatan("Mmm... sigo tomando nota.");
        return;
    }
    int ataque = azar(0, disponibles - 1);
    numAtaques++;                                 // La página anima cada ataque nuevo
    if (ataque == AVALANCHA) {
        ultimoAtaque = "Avalancha";
        decirJonatan("Traje a unos amigos. Tienen hambre.");
        for (int i = 0; i < GRUPOS_AVALANCHA; i++) llegaGrupo();
    } else if (ataque == TRAFICO) {
        ultimoAtaque = "Tráfico";
        decirJonatan("Qué pena, cerraron unas calles.");
        traficoPendiente++;
    } else if (ataque == CAOS) {
        ultimoAtaque = "Caos";
        decirJonatan("Uy, se me cayeron unas cajas.");
        int orden[NUM_CASILLAS_CAOS];             // Se escogen casillas de la lista fija sin repetir
        for (int i = 0; i < NUM_CASILLAS_CAOS; i++) orden[i] = i;
        int puestos = 0;
        for (int i = 0; i < NUM_CASILLAS_CAOS && puestos < OBSTACULOS_CAOS; i++) {
            int j = azar(i, NUM_CASILLAS_CAOS - 1);
            int aux = orden[i]; orden[i] = orden[j]; orden[j] = aux;
            int f = CASILLAS_CAOS[orden[i]][0], c = CASILLAS_CAOS[orden[i]][1];
            if (mapa.esPosicionSeguraParaObstaculo(f, c, jugadorF, jugadorC) && mapa.insertar(f, c, OBSTACULO, -1, TICKS_OBSTACULO)) puestos++;
        }
    } else {
        ultimoAtaque = "Auditoría";
        int n = cuentas.tamano();
        decirJonatan(n > 0 ? "Veamos esas cuentas sin cobrar." : "Todo cobrado... por ahora.");
        dinero -= COSTO_AUDITORIA * n;
        cambiarReputacion(-REP_AUDITORIA * n);
    }
    anotar("¡Ataque de Jonatan: " + ultimoAtaque + "!");
}

void Juego::cambiarReputacion(int cambio) {
    reputacion += cambio;
    if (reputacion > REP_MAXIMA) reputacion = REP_MAXIMA;
    if (reputacion <= 0 && faseActual != FASE_FINAL) {         // Se acabó la reputación: pierde
        reputacion = 0;
        decirJonatan("Cierren la cocina. Aquí no vuelvo.");
        finalizar(false);
    }
}

void Juego::finalizar(bool completo) {
    faseActual = FASE_FINAL;
    victoria = completo && reputacion >= REP_META && dinero >= DINERO_META;
    // Estrellas de la reseña de Jonatan (no cambian las reglas para ganar: solo cuentan qué tan cerca estuvo)
    if (victoria) estrellasFinal = 5;                                                       // Ganó
    else if (completo && (reputacion >= REP_META || dinero >= DINERO_META)) estrellasFinal = 4;   // Terminó y cumplió una de las dos metas
    else if (completo) estrellasFinal = 3;                                                  // Terminó sin cumplir ninguna
    else if (jornada >= 3) estrellasFinal = 2;                                              // Perdió por reputación desde la jornada 3
    else estrellasFinal = 1;                                                                // Perdió por reputación en la jornada 1 o 2
    decirJonatan(FRASES_FINAL[estrellasFinal - 1]);
    Puntaje p(nombreJugador, puntaje(), victoria);          // Entra al ranking en su lugar (de mayor a menor)
    int pos = 0;
    while (pos < ranking.tamano() && ranking[pos].puntos >= p.puntos) pos++;
    ranking.insertarEn(pos, p);
    puestoRanking = pos < 5 ? pos + 1 : 0;                  // Puesto para resaltarlo en la página (0 = no entró)
    while (ranking.tamano() > 5) ranking.eliminarUltimo();   // Solo los 5 mejores
    guardarRanking();
}

int Juego::puntaje() const {
    int p = dinero / 1000 + 10 * reputacion + BONO_NOCHE_PERFECTA * nochesPerfectas;
    return p < 0 ? 0 : p;
}

void Juego::decirJonatan(const string& frase) { fraseJonatan = frase; }

void Juego::anotar(const string& evento) {
    string cuando = faseActual == FASE_NOCHE ? "Noche " + to_string(tickNoche) : "Tick " + to_string(tick);
    eventos.agregar(cuando + ": " + evento);
    while (eventos.tamano() > 8) eventos.eliminarEn(0);       // Solo los 8 más recientes
}


// ============================================================
// Búsquedas pequeñas
// ============================================================

int Juego::buscarMesa(int idMesa) const {
    for (int i = 0; i < mesas.tamano(); i++) if (mesas[i].id == idMesa) return i;
    return -1;
}

int Juego::mesaDeCliente(int idCliente) const {
    for (int i = 0; i < mesas.tamano(); i++) if (mesas[i].ocupada && mesas[i].idCliente == idCliente) return i;
    return -1;
}

bool Juego::vecinoDe(int f, int c) const { return abs(jugadorF - f) + abs(jugadorC - c) == 1; }

int Juego::mesaVecina() const {
    for (int i = 0; i < mesas.tamano(); i++) if (vecinoDe(mesas[i].fila, mesas[i].col)) return i;
    return -1;
}

int Juego::mesaParaSiguiente() {                  // Para el jugador automático: mesa libre más pequeña donde cabe el que sigue
    Cliente c;
    bool deVip;
    if (faseActual != FASE_DIA || !siguienteCliente(c, deVip)) return -1;
    int mejor = -1;
    for (int i = 0; i < mesas.tamano(); i++) {
        const Mesa& m = mesas[i];
        if (!m.ocupada && m.capacidad >= c.personas && (mejor == -1 || m.capacidad < mesas[mejor].capacidad)) mejor = i;
    }
    return mejor == -1 ? -1 : mesas[mejor].id;
}

int Juego::azar(int minimo, int maximo) {
    if (maximo <= minimo) return minimo;
    return minimo + rand() % (maximo - minimo + 1);
}


// ============================================================
// Ranking (archivo ranking.txt: nombre;puntos;gano)
// ============================================================

void Juego::cargarRanking() {
    ranking.limpiar();
    if (!usaArchivo) return;
    ifstream archivo("ranking.txt");
    string linea;
    while (getline(archivo, linea) && ranking.tamano() < 5) {
        size_t a = linea.find(';');
        size_t b = linea.find(';', a + 1);
        if (a == string::npos || b == string::npos) continue;     // Línea dañada: se ignora
        ranking.agregar(Puntaje(linea.substr(0, a), atoi(linea.substr(a + 1, b - a - 1).c_str()), linea.substr(b + 1) == "1"));
    }
}

void Juego::guardarRanking() {
    if (!usaArchivo) return;
    ofstream archivo("ranking.txt");
    for (int i = 0; i < ranking.tamano(); i++)
        archivo << ranking[i].nombre << ';' << ranking[i].puntos << ';' << (ranking[i].gano ? 1 : 0) << '\n';
}


// ============================================================
// Estado en JSON para la página
// ============================================================

static string resumenJSON(const ResumenJornada& h) {      // Una jornada en JSON (hoy y el historial)
    ostringstream o;
    o << "{\"jornada\":" << h.jornada << ",\"atendidos\":" << h.atendidos << ",\"perdidos\":" << h.perdidos
      << ",\"ventas\":" << h.ventasDia << ",\"sinCobrar\":" << h.perdidoSinCobrar << ",\"noche\":" << h.ganadoNoche
      << ",\"entregas\":" << h.entregas << ",\"pedidos\":" << h.pedidos << ",\"reputacion\":" << h.reputacion << ",\"dinero\":" << h.dinero
      << ",\"repInicio\":" << h.repInicio << ",\"repFinDia\":" << h.repFinDia << ",\"aTiempo\":" << h.aTiempo << ",\"viajes\":" << h.viajes
      << ",\"rutasPerfectas\":" << h.rutasPerfectas << ",\"cierres\":" << h.cierres << ",\"urgentes\":" << h.urgentes << "}";
    return o.str();
}

string Juego::estadoJSON() {
    ostringstream o;
    const char* FASES[5] = {"inicio", "dia", "noche", "tienda", "final"};
    const Jornada& J = JORNADAS[jornada - 1];
    o << "{\"fase\":\"" << FASES[faseActual] << "\""
      << ",\"jornada\":" << jornada << ",\"nombreJornada\":" << comillas(J.nombre)
      << ",\"tick\":" << tick << ",\"ticksDia\":" << TICKS_DIA << ",\"ultimaLlamada\":" << (TICKS_DIA - ULTIMA_LLAMADA)
      << ",\"dinero\":" << dinero << ",\"reputacion\":" << reputacion << ",\"amenaza\":" << amenaza
      << ",\"metaDinero\":" << DINERO_META << ",\"metaRep\":" << REP_META
      << ",\"puntaje\":" << puntaje() << ",\"nochesPerfectas\":" << nochesPerfectas << ",\"gano\":" << (victoria ? "true" : "false")
      << ",\"estrellas\":" << estrellasFinal << ",\"puesto\":" << puestoRanking << ",\"numAtaques\":" << numAtaques
      << ",\"castigoMesa\":" << CASTIGO_MESA
      << ",\"jugador\":{\"f\":" << jugadorF << ",\"c\":" << jugadorC << ",\"nombre\":" << comillas(nombreJugador) << "}";

    // Plato en la mano
    if (manoCliente != -1) {
        int pos = mesaDeCliente(manoCliente);
        o << ",\"mano\":{\"mesa\":" << manoMesa << ",\"plato\":" << comillas(pos != -1 ? menu[mesas[pos].plato].nombre : "") << "}";
    } else o << ",\"mano\":null";

    // Mapa: se recorre la matriz dispersa fila por fila (solo casillas ocupadas)
    o << ",\"mapa\":[";
    bool primero = true;
    for (int f = 0; f < mapa.obtenerFilas(); f++) {
        for (const NodoCasilla* n = mapa.obtenerCabezaFila(f); n != NULL; n = n->siguiente) {
            o << (primero ? "" : ",") << "{\"f\":" << n->fila << ",\"c\":" << n->col << ",\"t\":" << (int)n->tipo
              << ",\"id\":" << n->idObjeto << ",\"v\":" << n->ticksRestantes << "}";
            primero = false;
        }
    }
    o << "],\"memoria\":{\"nodos\":" << mapa.obtenerTotalNodos() << ",\"casillas\":" << (FILAS_MAPA * COLUMNAS_MAPA)
      << ",\"dispersa\":" << (unsigned long)mapa.memoriaDispersaBytes() << ",\"densa\":" << (unsigned long)mapa.memoriaDensaEquivalenteBytes() << "}";

    // Mesas
    o << ",\"mesas\":[";
    for (int i = 0; i < mesas.tamano(); i++) {
        const Mesa& m = mesas[i];
        o << (i ? "," : "") << "{\"id\":" << m.id << ",\"cap\":" << m.capacidad << ",\"f\":" << m.fila << ",\"c\":" << m.col
          << ",\"ocupada\":" << (m.ocupada ? "true" : "false");
        if (m.ocupada) {
            o << ",\"personas\":" << m.personas << ",\"tipo\":" << m.tarjeta << ",\"plato\":" << comillas(menu[m.plato].nombre)
              << ",\"restante\":" << (m.tickLimite - tick) << ",\"paciencia\":" << (m.tickLimite - m.tickSentado)
              << ",\"comiendo\":" << (m.comiendoHasta >= 0 ? "true" : "false") << ",\"cocinando\":" << (m.cocinando ? "true" : "false")
              << ",\"cuenta\":" << m.cuenta;
        }
        o << "}";
    }
    o << "]";

    // Fila: primero la VIP (en orden de atención) y luego la común
    Cliente sig;
    bool sigVip = false;
    bool haySig = siguienteCliente(sig, sigVip);
    o << ",\"fila\":[";
    primero = true;
    Cliente c;
    for (int i = 0; filaVIP.obtenerEn(i, c); i++) {
        o << (primero ? "" : ",") << "{\"turno\":" << c.turno << ",\"nombre\":" << comillas(c.nombre) << ",\"personas\":" << c.personas
          << ",\"tipo\":" << c.tarjeta << ",\"espera\":" << (c.llegada + PACIENCIA_FILA - tick)
          << ",\"siguiente\":" << (haySig && c.turno == sig.turno ? "true" : "false") << "}";
        primero = false;
    }
    for (int i = 0; filaComun.obtenerEn(i, c); i++) {
        o << (primero ? "" : ",") << "{\"turno\":" << c.turno << ",\"nombre\":" << comillas(c.nombre) << ",\"personas\":" << c.personas
          << ",\"tipo\":" << c.tarjeta << ",\"espera\":" << (c.llegada + PACIENCIA_FILA - tick)
          << ",\"siguiente\":" << (haySig && c.turno == sig.turno ? "true" : "false") << "}";
        primero = false;
    }
    o << "],\"filaMaxima\":" << FILA_MAXIMA;

    // Pedidos en el montículo (se omiten los de grupos que ya se fueron)
    o << ",\"pedidos\":[";
    primero = true;
    ElementoMonticulo e;
    for (int i = 0; pedidos.obtenerEn(i, e); i++) {
        int pos = mesaDeCliente(e.dato);
        if (pos == -1 || mesas[pos].cocinando) continue;
        o << (primero ? "" : ",") << "{\"mesa\":" << mesas[pos].id << ",\"plato\":" << comillas(menu[mesas[pos].plato].nombre)
          << ",\"limite\":" << -e.clave << ",\"restante\":" << (-e.clave - tick) << ",\"posicion\":" << i << "}";
        primero = false;
    }
    o << "]";

    // Cocina y mesero
    if (cocinaCliente != -1) {
        int pos = mesaDeCliente(cocinaCliente);
        o << ",\"cocina\":{\"mesa\":" << (pos != -1 ? mesas[pos].id : 0) << ",\"plato\":" << comillas(pos != -1 ? menu[mesas[pos].plato].nombre : "")
          << ",\"faltan\":" << (cocinaListaEn - tick > 0 ? cocinaListaEn - tick : 0) << ",\"lista\":" << (tick >= cocinaListaEn ? "true" : "false") << "}";
    } else o << ",\"cocina\":null";
    o << ",\"mesero\":" << (meseroCliente != -1 ? "{\"mesa\":" + to_string(meseroMesa) + "}" : string("null"));

    // Cuentas pendientes
    int totalCuentas = 0;
    o << ",\"cuentas\":[";
    for (int i = 0; i < cuentas.tamano(); i++) {
        totalCuentas += cuentas[i].monto;
        o << (i ? "," : "") << "{\"mesa\":" << cuentas[i].mesa << ",\"monto\":" << cuentas[i].monto << "}";
    }
    o << "],\"totalCuentas\":" << totalCuentas;

    // Menú (lo desbloqueado) y su capacidad (para mostrar que es un arreglo dinámico)
    o << ",\"menu\":[";
    for (int i = 0; i < menu.tamano(); i++) {
        o << (i ? "," : "") << "{\"nombre\":" << comillas(menu[i].nombre) << ",\"precio\":" << menu[i].precio
          << ",\"prep\":" << menu[i].ticksPreparacion << "}";
    }
    o << "],\"capacidadMenu\":" << menu.obtenerCapacidad();

    // Mejoras
    o << ",\"mejoras\":[";
    for (int i = 0; i < NUM_MEJORAS; i++) {
        o << (i ? "," : "") << "{\"nombre\":" << comillas(NOMBRE_MEJORA[i]) << ",\"precio\":" << PRECIO_MEJORA[i]
          << ",\"comprada\":" << (mejoras[i] ? "true" : "false") << "}";
    }
    o << "],\"comproHoy\":" << (comproEstaJornada ? "true" : "false");

    // Noche: ciudad, calles (costo, cerrada, con tráfico), domicilios, distancias del GPS y el último viaje
    o << ",\"noche\":{\"tick\":" << tickNoche << ",\"duracion\":" << duracionNoche
      << ",\"estado\":\"" << (estadoNoche == NOCHE_DESVIO ? "desvio" : "base") << "\""
      << ",\"pos\":" << posMoto << ",\"gpsVisto\":" << (gpsVisto ? "true" : "false")
      << ",\"capacidad\":" << (mejoras[MEJORA_MOTO] ? 2 : 1) << ",\"cierresRestantes\":" << cierresRestantes
      << ",\"bonoRuta\":" << BONO_RUTA_PERFECTA << ",\"castigoSinEntregar\":" << CASTIGO_SIN_ENTREGAR << ",\"nodos\":[";
    for (int i = 0; i < ciudad.cantidadNodos(); i++) o << (i ? "," : "") << comillas(ciudad.nombreNodo(i));
    o << "],\"calles\":[";
    for (int i = 0; i < ciudad.cantidadCalles(); i++)
        o << (i ? "," : "") << "{\"a\":" << ciudad.extremoUno(i) << ",\"b\":" << ciudad.extremoDos(i) << ",\"costo\":" << ciudad.costoCalle(i)
          << ",\"cerrada\":" << (ciudad.estaCerrada(i) ? "true" : "false") << ",\"lenta\":" << (ciudad.esLenta(i) ? "true" : "false") << "}";
    o << "],\"domicilios\":[";
    for (int i = 0; i < domicilios.tamano(); i++) {
        const Domicilio& d = domicilios[i];
        bool enMoto = false;
        for (int k = 0; k < numCarga; k++) if (carga[k] == d.id) enMoto = true;
        o << (i ? "," : "") << "{\"id\":" << d.id << ",\"casa\":" << d.casa << ",\"nombreCasa\":" << comillas(ciudad.nombreNodo(d.casa))
          << ",\"limite\":" << d.limite << ",\"estado\":" << d.estado << ",\"llegada\":" << d.llegada
          << ",\"urgente\":" << (d.urgente ? "true" : "false") << ",\"pago\":" << d.pago << ",\"enMoto\":" << (enMoto ? "true" : "false") << "}";
    }
    o << "],\"dist\":[";                          // Distancias más cortas entre todos los lugares (Dijkstra desde cada uno)
    if (faseActual == FASE_NOCHE) {
        for (int i = 0; i < ciudad.cantidadNodos(); i++) {
            ciudad.calcularRutas(i);
            o << (i ? "," : "") << "[";
            for (int k = 0; k < ciudad.cantidadNodos(); k++) {
                int d = ciudad.distanciaA(k);
                o << (k ? "," : "") << (d >= INFINITO ? -1 : d);
            }
            o << "]";
        }
    }
    o << "],\"gps\":[";
    for (int i = 0; i < largoUltimaRuta; i++) o << (i ? "," : "") << ultimaRuta[i];
    o << "],\"recorrido\":[";
    for (int i = 0; i < largoRecorrido; i++) o << (i ? "," : "") << recorrido[i];
    o << "],\"entregas\":[";
    for (int i = 0; i < numEntregasRecientes; i++) o << (i ? "," : "") << "{\"id\":" << entregaId[i] << ",\"paso\":" << entregaPaso[i] << "}";
    o << "],\"calleCerrada\":" << calleCerradaAhora;
    if (hayOfertaUrgente) {
        ciudad.calcularRutas(RESTAURANTE);
        o << ",\"oferta\":{\"id\":" << ofertaUrgente.id << ",\"casa\":" << ofertaUrgente.casa << ",\"nombreCasa\":" << comillas(ciudad.nombreNodo(ofertaUrgente.casa))
          << ",\"limite\":" << ofertaUrgente.limite << ",\"pago\":" << PAGO_URGENTE << ",\"pagoTick\":" << PAGO_URGENTE_TICK
          << ",\"ida\":" << ciudad.distanciaA(ofertaUrgente.casa) << "}";
    } else o << ",\"oferta\":null";
    if (hayUltimoViaje) {
        o << ",\"ultimoViaje\":{\"costo\":" << ultimoCosto << ",\"optimo\":" << ultimoOptimo << ",\"perfecto\":" << (ultimoPerfecto ? "true" : "false")
          << ",\"bono\":" << ultimoBono << ",\"cierre\":" << (ultimoConCierre ? "true" : "false") << "}";
    } else o << ",\"ultimoViaje\":null";
    o << "},\"moto\":" << (mejoras[MEJORA_MOTO] ? "true" : "false");

    // Jonatan, eventos, hoy, historial y ranking
    o << ",\"jonatan\":{\"frase\":" << comillas(fraseJonatan) << ",\"ataque\":" << comillas(ultimoAtaque) << "}";
    o << ",\"eventos\":[";
    for (int i = eventos.tamano() - 1, k = 0; i >= 0; i--, k++) o << (k ? "," : "") << comillas(eventos[i]);    // El más reciente primero
    o << "]";
    o << ",\"hoy\":" << resumenJSON(hoy);
    o << ",\"historial\":[";
    for (int i = 0; i < historial.tamano(); i++) o << (i ? "," : "") << resumenJSON(historial[i]);
    o << "],\"ranking\":[";
    for (int i = 0; i < ranking.tamano(); i++)
        o << (i ? "," : "") << "{\"nombre\":" << comillas(ranking[i].nombre) << ",\"puntos\":" << ranking[i].puntos << ",\"gano\":" << (ranking[i].gano ? "true" : "false") << "}";
    o << "],\"ventasRegistradas\":" << ventas.tamano();

    bool salonVacio = filaComun.esvacia() && filaVIP.esvacia() && cuentas.estaVacio();
    for (int i = 0; i < mesas.tamano(); i++) if (mesas[i].ocupada) salonVacio = false;
    o << ",\"puedeCerrar\":" << (faseActual == FASE_DIA && tick >= TICKS_DIA - ULTIMA_LLAMADA && salonVacio ? "true" : "false");
    o << "}";
    return o.str();
}
