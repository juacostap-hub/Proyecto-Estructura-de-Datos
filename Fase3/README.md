# OrderStack: Restaurant Rush (Fase 3, Estructuras de Datos, C++)

Juego por turnos de 5 jornadas. De día se atiende el restaurante y de noche se reparten domicilios trazando la ruta en el mapa de la ciudad, mientras El Gran Crítico Jonatan sabotea el servicio. Al final, Jonatan publica su reseña de 1 a 5 estrellas. Las reglas completas están en `../Documentos/Restaurant_Rush_reglas_finales.docx`.

## Cómo compilar

Los archivos van sueltos en una misma carpeta.

Windows (MinGW, también el 6.3):

    g++ -std=c++17 -o proyecto *.cpp -lws2_32

Mac, Linux o Colab:

    g++ -std=c++17 -o proyecto *.cpp

## Cómo ejecutar

| Comando | Qué hace |
|---|---|
| `.\proyecto` (Windows) o `./proyecto` | Abre el juego en el navegador (http://localhost:8080) |
| `proyecto prueba` | Corre las pruebas automáticas: revisa las estructuras, revisa la noche (rutas, calles cerradas, partidas repetibles) y juega 600 partidas con un jugador automático |
| `proyecto sinnavegador` | Inicia el juego sin abrir el navegador (se entra a mano a http://localhost:8080) |

Controles de día: flechas para caminar, `E` para usar la cocina, una mesa o la caja, `Espacio` para esperar, clic en una mesa (o su número) para sentar al grupo que sigue, `H` para la ayuda.

Controles de noche: clic en un pedido (o en su casa) para elegirlo, clic en las esquinas iluminadas para trazar la ruta, `Retroceso` para borrar el último paso, **Completar regreso** para volver por el camino más corto y `Enter` (o **Salir con la moto**) para salir.

El ranking se guarda en `ranking.txt`, en la carpeta desde donde se ejecuta el programa.

## Estructuras de datos (todas hechas a mano)

| Estructura | Archivo | Uso en el juego |
|---|---|---|
| Arreglo dinámico (plantilla) | `ArregloDinamico.h` | Menú, mesas, cuentas pendientes, registro de ventas, historial, domicilios, ranking |
| Matriz dispersa | `MatrizDispersa.h` | Mapa del restaurante (solo las casillas ocupadas) |
| Montículo máximo | `MonticuloMax.h` | Pedidos de la cocina (clave = −tiempo límite) y Dijkstra (clave = −distancia) |
| Grafo + Dijkstra | `Grafo.h` | Ciudad de la noche: tiempo del GPS, ruta del GPS, desvíos cuando Jonatan cierra una calle y revisión de que ninguna casa quede aislada |
| Cola normal (fase 2) | `ColaComun.h` | Fila de clientes comunes |
| Cola con prioridad (fase 2) | `ColaVIP.h` | Fila VIP de 3 niveles |

## Archivos

- `main.cpp`: elige entre el juego y las pruebas.
- `Juego.h` / `Juego.cpp`: toda la lógica de la partida (sistema de ticks, día, noche, Jonatan, mejoras y ranking).
- `Restaurante.h`: reglas y números del juego, y las clases de datos (Plato, Mesa, Cuenta, Domicilio...).
- `Servidor.cpp`: servidor local con los sockets del sistema, sin librerías externas ni hilos (el mismo de la fase 2).
- `Pagina.h`: la página del juego (HTML, CSS y JavaScript). Solo dibuja: la lógica está en C++. Funciona sin internet (los dibujos son SVG y canvas hechos a mano).
- `Sprites.h`: las dos imágenes de Jonatan en caricatura (cuerpo y retrato), sin fondo, guardadas en base64.
- `Prueba.cpp`: pruebas automáticas.
- `Cliente.h`, `ColaComun.h`, `ColaVIP.h`: de la fase 2 (a `Cliente` se le agregó el tick de llegada).

Los cambios de esta versión (pantalla de inicio, cierre del día, noche con rutas propias, reseña final) están explicados en `CAMBIOS.md`.
