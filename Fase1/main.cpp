// SIMULADOR DE CAFETERÍA - Fase 1
// Punto de entrada del programa. Elige qué interfaz abrir:
//   ./proyecto            → interfaz gráfica (se abre en el navegador)
//   ./proyecto consola    → interfaz de consola (menús numerados)
//   ./proyecto grafica    → fuerza la interfaz gráfica
//   ./proyecto < datos.txt→ si los datos vienen de un archivo (Colab, pruebas), usa la consola automáticamente
// Opción extra: "sinnavegador" inicia la interfaz gráfica sin abrir el navegador.

#include <string>             // Para usar texto (std::string)

int ejecutarConsola();                          // Está en Consola.cpp
int ejecutarInterfazGrafica(bool abrirNavegador);    // Está en Servidor.cpp
bool entradaDesdeArchivo();                     // Está en Consola.cpp: dice si los datos no vienen del teclado

int main(int argc, char* argv[]) {
    bool consola = false, grafica = false, abrir = true;      // Qué se pidió por la línea de comandos
    for (int i = 1; i < argc; i++) {                          // Revisa cada opción escrita
        std::string a = argv[i];
        if (a == "consola") consola = true;
        else if (a == "grafica") grafica = true;
        else if (a == "sinnavegador") abrir = false;
    }
    if (consola) return ejecutarConsola();                    // Se pidió la consola
    if (grafica) return ejecutarInterfazGrafica(abrir);       // Se pidió la gráfica
    if (entradaDesdeArchivo()) return ejecutarConsola();      // Sin opciones: si los datos vienen de un archivo, consola
    return ejecutarInterfazGrafica(abrir);                    // Si no, interfaz gráfica
}
