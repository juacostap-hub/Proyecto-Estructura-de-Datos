// ORDERSTACK: RESTAURANT RUSH - Fase 3
// Punto de entrada del programa:
//   ./proyecto                → abre el juego en el navegador (interfaz gráfica)
//   ./proyecto prueba         → corre las pruebas automáticas (estructuras + cientos de partidas de un jugador automático)
//   ./proyecto sinnavegador   → inicia el servidor del juego sin abrir el navegador

#include <string>             // Para usar texto (std::string)

int ejecutarInterfazGrafica(bool abrirNavegador);    // Está en Servidor.cpp
int ejecutarPruebas();                               // Está en Prueba.cpp

int main(int argc, char* argv[]) {
    bool abrir = true;                                // Por defecto se abre el navegador
    for (int i = 1; i < argc; i++) {                  // Revisa cada opción escrita
        std::string a = argv[i];
        if (a == "prueba") return ejecutarPruebas();  // Solo pruebas
        if (a == "sinnavegador") abrir = false;
    }
    return ejecutarInterfazGrafica(abrir);            // El juego
}
