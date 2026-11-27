// PRUEBAS AUTOMÁTICAS: "proyecto prueba"
// 1. Revisa las estructuras de datos con casos conocidos (assert detiene el programa si algo falla).
// 2. Revisa la noche nueva: rutas válidas e inválidas, calles cerradas que nunca aíslan una casa,
//    y que la misma semilla con las mismas acciones dé siempre el mismo resultado.
// 3. Un jugador automático juega cientos de partidas completas con el código real del juego
//    y muestra cuántas gana según su habilidad. Sirve para confirmar el balance del documento de reglas.

#include "Juego.h"
#include <cassert>        // assert: detiene el programa si una condición no se cumple
#include <iostream>
#include <cstdlib>

using namespace std;

static void probarEstructuras() {
    ArregloDinamico<int> a;                       // Arreglo dinámico: crece, se copia y se achica
    for (int i = 0; i < 100; i++) a.agregar(i);
    assert(a.tamano() == 100 && a[99] == 99);
    ArregloDinamico<int> b = a;
    b[0] = -1;
    assert(a[0] == 0);                            // La copia no comparte memoria
    while (a.eliminarUltimo()) {}
    assert(a.obtenerCapacidad() == 10);           // Nunca baja de 10

    MonticuloMax m;                               // Montículo: con claves negativas sale primero el límite más cercano
    int limites[8] = {44, 12, 30, 7, 25, 90, 3, 61};
    for (int i = 0; i < 8; i++) m.insertar(ElementoMonticulo(i, -limites[i]));
    int anterior = -1000000;
    ElementoMonticulo e;
    while (m.extraer(e)) { assert(-e.clave >= anterior); anterior = -e.clave; }

    MatrizDispersa mapa;                          // Matriz dispersa: obstáculos que desaparecen
    assert(mapa.insertar(5, 7, OBSTACULO, -1, 2));
    assert(!mapa.esTransitable(5, 7));
    mapa.actualizarObstaculos();
    mapa.actualizarObstaculos();
    assert(mapa.esTransitable(5, 7) && mapa.obtenerTotalNodos() == 0);

    Grafo g;                                      // Dijkstra comparado con fuerza bruta en 300 sorteos de costos, con calles cerradas
    int cerradasTotal = 0;
    for (int r = 0; r < 300; r++) {
        g.sortearCostos();                        // (también abre todas las calles)
        int cerrar = r % 4;                       // 0 a 3 calles cerradas al azar (solo si no aíslan a nadie)
        for (int k = 0; k < cerrar; k++) {
            int c = rand() % g.cantidadCalles();
            if (g.sePuedeCerrar(c)) { g.cerrarCalle(c); cerradasTotal++; }
            assert(g.todoConectado());            // Nunca queda un lugar aislado
        }
        g.calcularRutas(RESTAURANTE);
        int d[MAX_NODOS];
        for (int i = 0; i < g.cantidadNodos(); i++) d[i] = INFINITO;
        d[0] = 0;
        for (int k = 0; k < g.cantidadNodos(); k++)
            for (int c = 0; c < g.cantidadCalles(); c++) {
                if (g.estaCerrada(c)) continue;   // La fuerza bruta también salta las calles cerradas
                int x = g.extremoUno(c), y = g.extremoDos(c), w = g.costoCalle(c);
                if (d[x] + w < d[y]) d[y] = d[x] + w;
                if (d[y] + w < d[x]) d[x] = d[y] + w;
            }
        for (int i = 0; i < g.cantidadNodos(); i++) assert(g.distanciaA(i) == d[i]);
    }
    assert(cerradasTotal > 50);
    g.abrirTodas();                               // Las casas 1, 3 y 5 tienen una sola calle: nunca se pueden cerrar
    assert(!g.sePuedeCerrar(g.calleEntre(5, 1)) && !g.sePuedeCerrar(g.calleEntre(7, 2)) && !g.sePuedeCerrar(g.calleEntre(9, 3)));
    cout << "  Estructuras: OK (arreglo dinámico, montículo, matriz dispersa, Dijkstra con calles cerradas)\n";
}

// ---------- Jugador automático ----------

static int pasoHacia(Juego& j, int objF, int objC) {     // Primer paso del camino más corto hacia una casilla vecina de (objF, objC)
    // BFS sobre el mapa con un arreglo fijo como fila (código de prueba, no es parte del juego)
    int dist[FILAS_MAPA][COLUMNAS_MAPA];
    int primero[FILAS_MAPA][COLUMNAS_MAPA];       // Dirección del primer paso que llevó a cada casilla
    for (int f = 0; f < FILAS_MAPA; f++) for (int c = 0; c < COLUMNAS_MAPA; c++) dist[f][c] = -1;
    int fila[FILAS_MAPA * COLUMNAS_MAPA][2];
    int ini = 0, fin = 0;
    const int df[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
    const char letra[4] = {'A', 'B', 'I', 'D'};
    int f0 = j.jugadorFila(), c0 = j.jugadorCol();
    dist[f0][c0] = 0;
    primero[f0][c0] = -1;
    fila[fin][0] = f0; fila[fin][1] = c0; fin++;
    while (ini < fin) {
        int f = fila[ini][0], c = fila[ini][1]; ini++;
        if (abs(f - objF) + abs(c - objC) == 1) return primero[f][c];    // Ya está al lado del objetivo
        for (int k = 0; k < 4; k++) {
            int nf = f + df[k], nc = c + dc[k];
            if (nf < 0 || nc < 0 || nf >= FILAS_MAPA || nc >= COLUMNAS_MAPA || dist[nf][nc] != -1 || !j.esTransitable(nf, nc)) continue;
            dist[nf][nc] = dist[f][c] + 1;
            primero[nf][nc] = primero[f][c] == -1 ? letra[k] : primero[f][c];
            fila[fin][0] = nf; fila[fin][1] = nc; fin++;
        }
    }
    return 0;                                     // No hay camino: espera
}

static void jugarDia(Juego& j, int desperdicio) {
    if (rand() % 100 < desperdicio) { j.esperar(); return; }         // Pierde un tick (duda, se equivoca)
    int mesa = j.mesaParaSiguiente();
    if (mesa != -1) { j.sentar(mesa); return; }   // Sienta al que sigue en la mesa más pequeña donde cabe
    if (j.cerrarTemprano().ok) return;
    int objF, objC;
    int mano = j.mesaDelPlatoEnMano();
    int t = j.tickActual();
    if (mano != -1) { objF = MESAS_INICIALES[mano - 1][2]; objC = MESAS_INICIALES[mano - 1][3]; }
    else if (j.cuentasPendientes() >= 3 || (j.cuentasPendientes() > 0 && t >= TICKS_DIA - ULTIMA_LLAMADA)) { objF = CAJA_F; objC = CAJA_C; }
    else { objF = COCINA_F; objC = COCINA_C; }
    int paso = pasoHacia(j, objF, objC);
    if (paso == -1) {                             // Ya está al lado: usa
        if (!j.interactuar().ok) j.esperar();
    } else if (paso == 0) j.esperar();
    else j.mover((char)paso);
}

struct Habilidad {                                // Cómo juega cada jugador automático
    const char* nombre;
    int desperdicio;                              // De día: de cada 100 acciones, cuántas pierde
    int errorRuta;                                // De noche: de cada 100 rutas, cuántas traza con un rodeo
    int usaGps;                                   // De noche: de cada 100 viajes, en cuántos mira el GPS (sin bono)
    int aceptaUrgente;                            // De cada 100 pedidos urgentes, cuántos acepta
};

static void conRodeo(Juego& j, int ruta[], int& largo) {    // Mete un rodeo en la ruta: va a una esquina vecina y vuelve
    if (largo < 3 || largo + 2 > MAX_RUTA) return;
    int i = 1 + rand() % (largo - 2);             // Un lugar del medio de la ruta
    int vecinos[MAX_NODOS], n = 0;
    for (int x = 0; x < 11; x++) if (x != ruta[i + 1] && j.costoCalle(ruta[i], x) > 0) vecinos[n++] = x;
    if (n == 0) return;
    int x = vecinos[rand() % n];
    for (int k = largo - 1; k > i; k--) ruta[k + 2] = ruta[k];     // Corre el resto dos lugares
    ruta[i + 1] = x;
    ruta[i + 2] = ruta[i];
    largo += 2;
}

struct DatosNoche { int noches, cierres, desvios, perfectas, viajes, urgentesOfrecidos, urgentesAceptados, aTiempo, entregas, pedidos; };

static void jugarNoche(Juego& j, const Habilidad& h, DatosNoche& dn) {
    int ruta[MAX_RUTA + 4], largo = 0;
    if (j.hayOferta()) {                          // Pedido urgente: lo acepta según su habilidad
        dn.urgentesOfrecidos++;
        // Lo piensa: lo acepta si después de llevarlo los demás pedidos todavía alcanzan a llegar a tiempo
        Domicilio u = j.oferta();
        int vuelta = u.limite - j.tickNocheActual() + 2;          // Más o menos lo que tarda ir y volver (ida + holgura)
        bool noEstorba = true;
        for (int i = 0; i < j.cantidadDomicilios(); i++) {
            Domicilio d = j.domicilio(i);
            if (d.estado != DOM_PENDIENTE) continue;
            int ida = j.duracionViaje(d.id, -1) / 2;
            bool alcanzaHoy = j.tickNocheActual() + ida <= d.limite;
            if (alcanzaHoy && j.tickNocheActual() + vuelta + ida > d.limite) noEstorba = false;
        }
        bool acepta = rand() % 100 < h.aceptaUrgente && (noEstorba || h.aceptaUrgente < 60);
        if (acepta) dn.urgentesAceptados++;
        assert(j.responderUrgente(acepta).ok);
        return;
    }
    if (j.enDesvio()) {                           // Jonatan cerró una calle: traza un desvío
        dn.desvios++;
        assert(!j.terminarNoche().ok);            // No puede terminar la noche con la moto en la calle
        assert(j.rutaOptima(-1, -1, ruta, largo));
        if (rand() % 100 < h.errorRuta) conRodeo(j, ruta, largo);
        Resultado r = j.desviar(ruta, largo);
        assert(r.ok);
        return;
    }
    int mejor = -1, mejorSeg = -1;                // Prefiere el que alcanza a llegar a tiempo y vence primero; si ninguno alcanza, el más cercano
    bool mejorAlcanza = false;
    int limMejor = 0, durMejor = 0;
    for (int i = 0; i < j.cantidadDomicilios(); i++) {
        Domicilio d = j.domicilio(i);
        if (d.estado != DOM_PENDIENTE) continue;
        int dur = j.duracionViaje(d.id, -1);
        bool alcanza = j.tickNocheActual() + dur / 2 <= d.limite;
        bool mejorQue = mejor == -1 || (alcanza && !mejorAlcanza) ||
                        (alcanza == mejorAlcanza && (alcanza ? d.limite < limMejor : dur < durMejor));
        if (mejorQue) { mejor = d.id; mejorAlcanza = alcanza; limMejor = d.limite; durMejor = dur; }
    }
    if (mejor != -1 && j.tieneMejora(MEJORA_MOTO)) {                  // Con moto: el segundo que haga la ruta más corta
        int mejorDur = 1000000;
        for (int i = 0; i < j.cantidadDomicilios(); i++) {
            Domicilio d = j.domicilio(i);
            if (d.estado != DOM_PENDIENTE || d.id == mejor) continue;
            int dur = j.duracionViaje(mejor, d.id);
            if (dur < mejorDur) { mejorDur = dur; mejorSeg = d.id; }
        }
    }
    for (int intento = 0; intento < 2 && mejor != -1; intento++) {    // Primero con los dos pedidos; si no alcanza, con uno
        int seg = intento == 0 ? mejorSeg : -1;
        if (intento == 1 && mejorSeg == -1) break;
        if (!j.rutaOptima(mejor, seg, ruta, largo)) continue;
        if (rand() % 100 < h.usaGps) assert(j.verRuta(mejor, seg).ok);    // Mira el GPS: ese viaje no da bono
        else if (rand() % 100 < h.errorRuta) conRodeo(j, ruta, largo);    // Se equivoca: traza un rodeo
        if (j.salir(mejor, seg, ruta, largo).ok) return;
    }
    j.terminarNoche();
}

static void jugarTienda(Juego& j) {
    const int orden[NUM_MEJORAS] = {MEJORA_COCINA, MEJORA_MESERO, MEJORA_MOTO, MEJORA_AMBIENTE};
    for (int i = 0; i < NUM_MEJORAS; i++) if (j.comprarMejora(orden[i]).ok) break;
    j.siguienteJornada();
}

static unsigned long partida(Juego& j, unsigned semilla, const Habilidad& h, DatosNoche& dn) {   // Devuelve una "huella" de la partida
    j.nuevaPartida("Prueba", semilla);
    int pasos = 0;
    while (j.fase() != FASE_FINAL) {
        assert(++pasos < 100000);                 // Nunca se queda atascado
        int r = j.reputacionActual();
        assert(r >= 0 && r <= REP_MAXIMA);        // La reputación siempre está entre 0 y 100
        if (j.fase() == FASE_DIA) { assert(j.tickActual() <= TICKS_DIA); jugarDia(j, h.desperdicio); }
        else if (j.fase() == FASE_NOCHE) {
            assert(j.tickNocheActual() <= j.duracionNocheActual() + 60);    // Un desvío puede pasarse un poco de la noche, nunca mucho
            jugarNoche(j, h, dn);
            if (j.fase() != FASE_NOCHE) {         // Terminó la noche: se anotan sus datos
                ResumenJornada hoy = j.resumenHoy();
                dn.noches++; dn.cierres += hoy.cierres; dn.perfectas += hoy.rutasPerfectas; dn.viajes += hoy.viajes;
                dn.aTiempo += hoy.aTiempo; dn.entregas += hoy.entregas; dn.pedidos += hoy.pedidos;
            }
        }
        else if (j.fase() == FASE_TIENDA) jugarTienda(j);
    }
    assert(j.estrellas() >= 1 && j.estrellas() <= 5);
    assert(j.gano() == (j.estrellas() == 5));
    string json = j.estadoJSON();                 // El JSON se arma sin fallar al final de la partida
    unsigned long huella = 5381;                  // Huella del estado final (para comparar dos partidas)
    for (size_t i = 0; i < json.size(); i++) huella = huella * 33 + (unsigned char)json[i];
    return huella;
}

static void probarNoche() {
    // Lleva una partida hasta la primera noche con el jugador experto
    Juego j(false);
    j.nuevaPartida("Prueba", 77);
    while (j.fase() == FASE_DIA) jugarDia(j, 0);
    assert(j.fase() == FASE_NOCHE && j.cantidadDomicilios() == 2);
    int a = j.domicilio(0).id, casa = j.domicilio(0).casa;
    int ruta[MAX_RUTA + 2], largo = 0;
    assert(j.rutaOptima(a, -1, ruta, largo) && ruta[0] == RESTAURANTE && ruta[largo - 1] == RESTAURANTE);

    int malInicio[3] = {1, 0, 1};                 // Rutas que se deben rechazar
    assert(!j.salir(a, -1, malInicio, 3).ok);                     // No empieza en el restaurante
    int saltando[3] = {0, casa, 0};
    assert(j.costoCalle(0, casa) == -1 && !j.salir(a, -1, saltando, 3).ok);   // No hay calle directa del restaurante a una casa
    int sinCasa[3] = {0, 1, 0};
    assert(!j.salir(a, -1, sinCasa, 3).ok);                       // No pasa por la casa del pedido
    int larga[MAX_RUTA + 1];
    for (int i = 0; i <= MAX_RUTA; i++) larga[i] = (i % 2) ? 1 : 0;
    assert(!j.salir(a, -1, larga, MAX_RUTA + 1).ok);              // Demasiado larga
    assert(!j.salir(a, j.domicilio(1).id, ruta, largo).ok);      // Dos pedidos sin la moto mejorada
    assert(!j.desviar(ruta, largo).ok);                           // No está en desvío

    int dinero = j.dineroActual();                // La ruta óptima sí sale, entrega el pedido y da el bono de ruta perfecta
    assert(j.salir(a, -1, ruta, largo).ok);
    assert(j.domicilio(0).estado != DOM_PENDIENTE && j.posicionMoto() == RESTAURANTE);
    assert(j.dineroActual() >= dinero + BONO_RUTA_PERFECTA + PAGO_TARDE);
    assert(j.resumenHoy().rutasPerfectas == 1);

    int b = j.domicilio(1).id;                    // Con el GPS mirado, el viaje ya no da bono
    assert(j.rutaOptima(b, -1, ruta, largo) && j.verRuta(b, -1).ok);
    assert(j.salir(b, -1, ruta, largo).ok);
    assert(j.fase() == FASE_TIENDA);              // Entregó todo: la noche termina sola
    assert(j.resumenHoy().rutasPerfectas == 1);

    // Misma semilla + mismas acciones = mismo resultado (se juegan dos partidas idénticas)
    Habilidad bueno = {"", 10, 15, 10, 80};
    for (int s = 0; s < 20; s++) {
        DatosNoche d1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, d2 = d1;
        Juego j1(false), j2(false);
        unsigned long h1 = partida(j1, 500 + s, bueno, d1);
        unsigned long h2 = partida(j2, 500 + s, bueno, d2);
        assert(h1 == h2 && j1.puntaje() == j2.puntaje());
    }
    cout << "  Noche: OK (rutas inválidas rechazadas, bono de ruta perfecta, GPS sin bono, partidas repetibles)\n";
}

int ejecutarPruebas() {
    cout << "\n  PRUEBAS DE RESTAURANT RUSH\n";
    probarEstructuras();
    probarNoche();
    const int PARTIDAS = 200;
    const Habilidad jugadores[3] = {
        {"Experto (no pierde ticks)",       0,  0,  0, 100},
        {"Bueno (pierde 1 de cada 10)",    10, 15, 10,  80},
        {"Regular (pierde 2 de cada 10)",  20, 35, 30,  50}
    };
    int desviosTotal = 0, urgentesTotal = 0;
    for (int h = 0; h < 3; h++) {
        int ganadas = 0, completas = 0;
        int estrellas[6] = {0, 0, 0, 0, 0, 0};
        long dineroTotal = 0, repTotal = 0;
        DatosNoche dn = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        for (int p = 0; p < PARTIDAS; p++) {
            Juego j(false);                       // Juego nuevo que no lee ni escribe ranking.txt
            partida(j, 1000 + p, jugadores[h], dn);
            if (j.gano()) ganadas++;
            if (j.jornadaActual() == NUM_JORNADAS && j.reputacionActual() > 0) completas++;
            estrellas[j.estrellas()]++;
            dineroTotal += j.dineroActual();
            repTotal += j.reputacionActual();
        }
        desviosTotal += dn.desvios;
        urgentesTotal += dn.urgentesOfrecidos;
        cout << "  " << jugadores[h].nombre << ": gana " << (100 * ganadas / PARTIDAS) << "%, termina las 5 jornadas "
             << (100 * completas / PARTIDAS) << "%, dinero promedio $" << dineroTotal / PARTIDAS
             << ", reputacion promedio " << repTotal / PARTIDAS << "\n";
        cout << "      Estrellas (1 a 5): " << estrellas[1] << " / " << estrellas[2] << " / " << estrellas[3] << " / " << estrellas[4] << " / " << estrellas[5]
             << ". Noche: " << (dn.pedidos ? 100 * dn.aTiempo / dn.pedidos : 0) << "% a tiempo, "
             << (dn.viajes ? 100 * dn.perfectas / dn.viajes : 0) << "% rutas perfectas, "
             << dn.cierres << " calles cerradas en " << dn.noches << " noches, "
             << dn.urgentesAceptados << " urgentes aceptados de " << dn.urgentesOfrecidos << "\n";
    }
    assert(desviosTotal > 0 && urgentesTotal > 0);    // Las partidas sí pasan por desvíos y pedidos urgentes
    cout << "  Todas las pruebas pasaron.\n\n";
    return 0;
}
