# Simulador de Cafetería (Estructuras de Datos, C++)

Simulador de una cafetería con salón y domicilios. Las estructuras de datos están programadas a mano:

| Estructura | Dónde se usa | Archivo |
|---|---|---|
| Arreglos dinámicos | Menú y mesas | `Proyecto1ED.h` |
| Pila | Comanda de cada mesa y del domicilio | `Proyecto1ED.h` |
| Cola normal | Fila de espera común | `ColaComun.h` |
| Cola con prioridad | Fila VIP | `ColaVIP.h` |
| Cola circular | Turnos de meseros y domiciliarios | `ColaCircular.h`, `GrupoPersonal.*` |

## Cómo compilar

Windows (PowerShell, con g++ de MinGW o MSYS2):

    g++ -std=c++17 -o proyecto *.cpp -lws2_32

Mac, Linux o Colab:

    g++ -std=c++17 -o proyecto *.cpp

## Cómo ejecutar

| Comando | Qué abre |
|---|---|
| `.\proyecto` (Windows) o `./proyecto` | **Interfaz gráfica**: se abre sola en el navegador (http://localhost:8080) |
| `proyecto consola` | Interfaz de consola (menús numerados) |
| `proyecto < entrada.txt` | Consola con las respuestas leídas de un archivo (Colab, pruebas) |

La interfaz gráfica es una página que muestra el estado del programa. **Toda la lógica y las estructuras están en C++**:
el programa abre un servidor local (`Servidor.cpp`, hecho con los sockets del sistema, sin librerías externas) y cada botón llama a una operación de `Cafeteria.cpp`.

## Archivos

- `main.cpp`: elige la interfaz. `Consola.cpp`: interfaz de consola. `Servidor.cpp` y `Pagina.h`: interfaz gráfica.
- `Cafeteria.h/.cpp`: operaciones de la interfaz gráfica. `Estado.h`: datos y reglas compartidas.

No se usa ninguna librería externa: la única parte de la librería estándar que se usa es `string` (y flujos de texto) para el texto.
