# Cambios de esta versión (mejoras visuales y de jugabilidad)

Se compila y se ejecuta igual que antes:

    g++ -std=c++17 -o proyecto *.cpp -lws2_32
    proyecto prueba

## Qué archivos cambiaron

| Archivo | Qué cambió | Por qué |
|---|---|---|
| `Pagina.h` | Página nueva completa: inicio animado, mapa en caricatura, cierre del día, noche con rutas trazadas por el jugador y reseña final | Partes 1 a 5 |
| `Sprites.h` | Imágenes nuevas de Jonatan (caricatura sin fondo, buena resolución) | Parte 3 |
| `Juego.h`, `Juego.cpp` | Noche nueva (rutas del jugador, GPS, calles cerradas, pedido urgente), estrellas de la reseña, datos nuevos para la página | Parte 4 y 5: es lógica del juego, así que va en C++ |
| `Grafo.h` | Se le agregó "cerrar calle", "calle con tráfico" y "¿todo sigue conectado?". Dijkstra salta las calles cerradas | Parte 4. No se reemplazó nada: sigue la lista de adyacencia, el montículo y Dijkstra del grupo |
| `Restaurante.h` | Números nuevos de la noche y datos nuevos de cada jornada (a tiempo, viajes, rutas perfectas...) | Parte 4 |
| `Servidor.cpp` | Rutas nuevas: `/api/salir`, `/api/desviar`, `/api/urgente` (y `/api/ruta` ahora es el GPS) | Parte 4 |
| `Prueba.cpp` | El jugador automático aprende la noche nueva y hay pruebas nuevas | Para que `proyecto prueba` revise todo |
| `README.md` | Controles de noche y descripción nueva | |

No se usa `vector`, `stack` ni `queue`, no hay hilos y no hay tildes en nombres de variables ni funciones. Se revisó que compile en C++11, C++14 y C++17.

## Parte 1: el cierre del día (solo en la página)

- El cielo de las ventanas, la luz del piso y todo el salón se ponen naranja y luego morado en los últimos 30 ticks. Al final se prenden las lámparas.
- Avisos grandes sobre el mapa: **Atardece** (quedan 30), **Última llamada** (quedan 25: ya no entran clientes y el letrero de la puerta pasa a CERRADO), **Cierre en 15** y **Cierre en 5**. Cuando sale un aviso, se borran las teclas que estaban en fila y las teclas del juego se pausan un segundo, para que nadie se pase del cierre sin verlo.
- Reloj nuevo: un sol que cruza el cielo y baja, y una barra verde, ámbar, roja y roja oscura.
- Panel **"Si el día cierra ahora pierdes"**: cuentas en la caja, mesas comiendo, mesas sin servir (−6 cada una) y a cuántos pasos está la caja. La caja late con un anillo amarillo.
- Al cerrar: el sol se oculta, sale la luna, se prenden las ventanas de la ciudad y aparece el resumen (ventas, reputación al abrir y al cerrar, atendidos, perdidos, dinero sin cobrar) con el botón **Empezar la noche**.

**Cómo probarla:** juega la jornada 1 y deja unas cuentas sin cobrar. Para llegar rápido al final del día, deja presionada la barra espaciadora: se detiene un segundo en cada aviso.

## Parte 2: pantalla de inicio (solo en la página)

Estilo pantalla de carga: rayos que giran, la cafetería OrderStack, el mesero corriendo con su torre de platos, tres clientes asustados, comida volando, Jonatan grande con el sello CRÍTICO, el título con contorno grueso y la pizarra **Los mejores** con el ranking. La barra de carga cuenta la historia en tres frases y al llegar a 100 % se convierte en el botón **JUGAR**. Todo está dibujado con SVG y CSS dentro de la página: no necesita internet.

**Cómo probarla:** abre el juego, espera la historia o haz clic para saltarla, pulsa JUGAR y escribe tu nombre.

## Parte 3: imagen de Jonatan (Sprites.h y la página)

- Se le quitó el fondo blanco a la imagen nueva sin dejar borde blanco. Hay dos versiones en PNG: cuerpo (537 × 820) y retrato (288 × 288), unos 80 KB entre las dos. Se muestran a la mitad de su tamaño, así se ven nítidas en cualquier pantalla.
- Se quitaron todos los `image-rendering: pixelated`.
- Para que no choque con el mapa, **todo el juego pasó a estilo caricatura**: el restaurante y la ciudad se dibujan con formas suaves y contorno oscuro grueso, con los mismos colores de OrderStack. El mesero es el mismo de la pantalla de inicio y los clientes cambian de cara según su paciencia (feliz, serio, enojado con vapor).
- La imagen corta a Jonatan a la altura de los muslos. En el inicio queda detrás de un mostrador para que no se note.
- Extra: cuando Jonatan ataca, entra en grande con el nombre del ataque (lo mismo cuando cierra una calle de noche).

**Cómo probarla:** mira el inicio, la esquina de Jonatan y la reseña final. Para ver un ataque, en la jornada 2 sienta grupos y no les sirvas: cada grupo que se va de la mesa sube la amenaza 10, y al llegar a 100 Jonatan ataca.

## Parte 4: noche de domicilios (C++ y página)

1. Eliges 1 pedido (2 con la moto mejorada) en la lista o haciendo clic en su casa.
2. Trazas la ruta haciendo clic en las esquinas iluminadas. Ves cuánto tarda tu ruta, cuánto tarda la del **GPS (Dijkstra)** y si cada pedido llega caliente o frío.
3. **Completar regreso** termina el camino de vuelta por la ruta más corta.
4. Si tu ruta es igual de rápida que la del GPS, ganas **$5.000 de bono de ruta perfecta**. Si pulsas **Ver GPS**, la página dibuja la ruta, pero ese viaje ya no da bono.
5. La moto recorre la ruta calle por calle. Desde la jornada 2, Jonatan puede **cerrar la calle que tienes adelante**: la moto se detiene en esa esquina y trazas un desvío (el GPS recalcula desde ahí con Dijkstra). Antes de cerrar una calle, el juego revisa con Dijkstra que ninguna casa quede aislada.
6. Desde la jornada 2, al volver del primer viaje puede llegar un **pedido urgente** que paga el doble. Lo aceptas o no.

**Por qué hubo que tocar C++:** antes C++ calculaba la ruta y hacía todo el viaje de una vez. Ahora tiene que recibir la ruta del jugador, revisar que cada calle exista y esté abierta, mover la moto calle por calle gastando ticks, cerrar calles y volver a correr Dijkstra. Todo eso es lógica del juego. Los ticks de la noche también pasan por `avanzarTick()`: cada calle cuesta tantos ticks como su costo. Todo el azar usa la semilla, así que el mismo orden de acciones da siempre el mismo resultado (`proyecto prueba` lo comprueba jugando 20 partidas dos veces y comparando).

**Cómo probarla:** en la noche 1 traza una ruta y compárala con el GPS. En la noche 2 o siguientes vas a ver la calle cerrada y el pedido urgente.

## Parte 5: final de la noche y del juego

- Reloj de la noche con una luna que cruza el cielo y la misma barra de colores.
- Cuando ningún pedido alcanza ida y vuelta, aparece **"Se acabó el tiempo"** y un panel con la penalización (−5 por pedido) y cómo queda la reputación. Si con eso pierdes, lo dice en rojo.
- **Terminar la noche** pide confirmación y muestra la penalización antes.
- Resumen de la noche: calientes, fríos, sin entregar, dinero, rutas perfectas y reputación.
- Final: la reseña de Jonatan sale en **La Gaceta del Sabor**. Las estrellas caen una por una, luego el sello (RECOMENDADO, CASI, REGULAR o NO RECOMENDADO) y su frase. Abajo están los números, el historial y el ranking con tu puesto resaltado.

| Resultado | Estrellas | Frase de Jonatan |
|---|---|---|
| Ganar | 5 | «Cinco estrellas. No se lo digan a nadie.» |
| Terminar las 5 jornadas cumpliendo una de las dos metas | 4 | «Cuatro estrellas. Casi me convencen.» |
| Terminar sin cumplir ninguna | 3 | «Tres estrellas... y estoy siendo generoso.» |
| Perder por reputación desde la jornada 3 | 2 | «Cierren la cocina. Aunque el café... no estaba mal.» |
| Perder por reputación en la jornada 1 o 2 | 1 | «Cierren la cocina. Aquí no vuelvo.» |

**Cómo probarla:** termina una partida, o pierde a propósito esperando sin atender a nadie.

## Pruebas: antes y después (`proyecto prueba`, 200 partidas por jugador)

| Jugador automático | Antes: gana / reputación | Ahora: gana / reputación |
|---|---|---|
| Experto | 100 % / 95 | 100 % / 92 |
| Bueno | 97 % / 91 | 97 % / 84 |
| Regular | 80 % / 68 | 75 % / 53 |

La noche ahora depende de la habilidad: el experto siempre iguala al GPS y el regular se equivoca en 35 de cada 100 rutas. Antes llegaba a tiempo el 81 % de los domicilios; ahora entre 63 % (regular) y 72 % (experto), por las calles cerradas y los urgentes. El jugador bueno gana igual que antes, así que el balance general se mantiene. Además, la página se probó en Chrome automático: partidas completas ganadas y perdidas, sin errores de JavaScript.

## Texto para actualizar el documento de reglas

**Tabla "Resumen y cambios" (filas nuevas):**

| Qué cambia | Antes | Ahora | Por qué |
|---|---|---|---|
| Ruta de los domicilios | Dijkstra la elegía sola | El jugador la traza; el GPS (Dijkstra) dice el mejor tiempo | La noche tiene decisiones reales |
| Bono | Noche perfecta | Noche perfecta + ruta perfecta ($5.000 por viaje) | Igualar al GPS ahora es mérito del jugador |
| Calles cerradas | No había | Jonatan cierra 1 calle por noche desde la jornada 2 (2 en la jornada 5) | Obliga a recalcular la ruta |
| Pedido urgente | No había | Uno por noche desde la jornada 2, opcional, paga el doble | Decisión de riesgo |
| Reseña final | 5 o 3 estrellas | De 1 a 5 estrellas según qué tan cerca quedó | Final más claro |

**Historia y villano.** Frases nuevas:
- Calle cerrada: «Uy, esa calle está en obras. Qué pena.»
- Ruta perfecta: «Hmm. Ni yo conozco tan bien estas calles.»
- Pedido urgente: «¿Un pedido urgente? Veamos si llega caliente.»
- Final de 4 estrellas: «Cuatro estrellas. Casi me convencen.»
- Final perdido desde la jornada 3: «Cierren la cocina. Aunque el café... no estaba mal.»

Cambiar el párrafo de "Gráficos" por: *La imagen de Jonatan es una caricatura sin fondo, en dos versiones: cuerpo completo de 537×820 píxeles (inicio, ataques y reseña final) y retrato de 288×288 (esquina con sus frases). Se muestran a la mitad de su tamaño para que se vean nítidas. El restaurante, la ciudad y los personajes se dibujan en el mismo estilo caricatura, con contorno oscuro grueso y los colores de OrderStack.*

**Cómo se gana.** Agregar: *Al terminar, Jonatan publica una reseña: 5 estrellas si se gana; 4 si se terminan las 5 jornadas cumpliendo una de las dos metas; 3 si se terminan sin cumplir ninguna; 2 si la reputación llega a 0 desde la jornada 3; 1 si llega a 0 en la jornada 1 o 2.*

**El tiempo y los controles.** Cambiar la fila de domicilios por: *Hacer un domicilio (noche) | Elegir el pedido, trazar la ruta con clics en las esquinas y pulsar Salir | El costo de la ruta trazada (cada calle cuesta sus ticks)*. Agregar: *Al cerrar el día, cada grupo sentado que no alcanzó a recibir su plato se va y resta 6 de reputación. Los que estaban comiendo se van sin pagar y su cuenta se pierde, igual que las cuentas sin cobrar. De noche, avanzarTick() avanza el reloj de la noche: la moto gasta tantos ticks como el costo de cada calle que recorre.*

**La noche (reemplaza la sección):**
- De noche se reparten entre 2 y 5 domicilios en 60 ticks (más los que sobraron del día). Cada uno tiene una casa y un tiempo límite entre 15 y 45 ticks.
- El jugador elige 1 pedido (2 con la moto mejorada) y traza la ruta con clics: debe salir del restaurante, pasar por la casa de cada pedido y volver. El viaje dura lo que suman las calles de su ruta. Solo puede salir si alcanza a volver antes de que termine la noche.
- El GPS muestra cuánto tarda la ruta más rápida (Dijkstra). Si la ruta del jugador es igual de rápida, gana $5.000 de bono de ruta perfecta. Si mira la ruta del GPS, ese viaje no da bono.
- Desde la jornada 2, Jonatan cierra la calle que la moto tiene adelante: 1 vez por noche en las jornadas 2 a 4 y 2 veces en la 5, a partir de un tick al azar entre el 4 y el 16 (y la segunda, de 8 a 16 ticks después). Nunca cierra una calle que deje un lugar aislado (se revisa con Dijkstra). La moto se detiene en esa esquina y el jugador traza un desvío; si el desvío lo hace pasar del final de la noche, termina el viaje igual y los pedidos que pasen de su límite llegan tarde. Las calles cerradas se abren la noche siguiente.
- Desde la jornada 2, una vez por noche, al volver del primer viaje puede llegar un pedido urgente a una casa sin pedido (solo si hay tiempo para ir y volver). Su límite es el tick actual + la ida más corta + 2 a 5 ticks. Paga $30.000 + $2.000 por cada tick que sobre; tarde paga $15.000 y resta 3 de reputación; sin entregar resta 5. Se puede rechazar sin castigo.
- La noche termina sola cuando se entregan todos los pedidos. Si el jugador la termina antes, cada pedido sin entregar resta 5 de reputación (el juego lo muestra antes de confirmar).

**Jonatan.** Agregar: *De noche, además, Jonatan cierra calles (ver La noche). Eso no depende de la barra de amenaza.*

**Las cinco jornadas.** Agregar a la tabla la columna "Calles cerradas de noche": 0, 1, 1, 1, 2.

**Estructuras de datos.** Fila de Grafo + Dijkstra: *Calles de la ciudad con lista de adyacencia. Dijkstra (con el mismo montículo, clave −distancia) calcula el tiempo y la ruta del GPS, recalcula desde la esquina donde quedó la moto cuando Jonatan cierra una calle, revisa que ninguna casa quede aislada antes de cerrarla y da a la página las distancias entre todos los lugares. Salta las calles cerradas.*

**Calibración.** Agregar la tabla de "Pruebas: antes y después" de este archivo.

## Notas

- Aquí no se pudo probar con MinGW 6.3, solo con g++ en C++11, C++14 y C++17. Si al compilar en Windows sale algún error, cópialo completo y se corrige.
- `Pagina.h` pesa más que antes (unos 135 KB), porque tiene todos los dibujos. `Sprites.h` pesa unos 115 KB. Las imágenes van partidas en líneas cortas que C++ une solas al compilar.
