#ifndef CAFETERIA_H       // Evita que este archivo se lea más de una vez al compilar
#define CAFETERIA_H       // Marca que este archivo ya fue leído

#include <string>
#include "Estado.h"       // Estructuras de las tres personas y reglas compartidas

// NÚCLEO DE LA INTERFAZ GRÁFICA
// La clase Cafeteria junta todas las estructuras y ofrece una operación por cada botón de la interfaz.
// Ninguna operación imprime ni lee del teclado: devuelven un Resultado (ok + mensaje) y el estado
// completo del programa se entrega en formato JSON para que la página lo dibuje.

struct Resultado {                // Lo que devuelve cada operación
    bool ok;                      // true = se hizo; false = no se pudo (el mensaje dice por qué)
    std::string mensaje;          // Texto para mostrar al usuario
    std::string extra;            // Datos adicionales en JSON (por ejemplo, la cuenta pagada); puede estar vacío
};

class Cafeteria {
  public:
    GrupoPersonal meseros;            // Cola circular de meseros (Persona 3)
    GrupoPersonal domiciliarios;      // Cola circular de domiciliarios (Persona 3)
    FilasEspera filas;                // Fila común (cola normal) y fila VIP (cola con prioridad) (Persona 2)
    Restaurante rest;                 // Menú, mesas (arreglos dinámicos) y comandas (pilas) (Persona 1)

    Cafeteria();                                                           // Crea la cafetería con los datos iniciales

    Resultado registrarCliente(const std::string& nombre, int personas, int tarjeta);    // tarjeta: 0 = sin tarjeta, 1 a 3 = VIP
    Resultado llamarSiguiente();                                           // Llama al siguiente cliente y le da mesa y mesero
    Resultado tomarComanda(int idMesa, int indicePlato);                   // Apila un producto en la comanda de la mesa
    Resultado deshacerProducto(int idMesa);                                // Desapila el último producto de la mesa
    Resultado pagarCuenta(int idMesa);                                     // Cobra la comanda, libera mesa y mesero

    Resultado agregarProductoDomicilio(int indicePlato);                   // Apila un producto en el pedido a domicilio
    Resultado deshacerDomicilio();                                         // Desapila el último producto del pedido
    Resultado confirmarDomicilio(const std::string& cliente, const std::string& direccion);    // Cobra y asigna domiciliario

    Resultado agregarPersonal(bool esMesero, const std::string& nombre);   // Agrega un mesero o un domiciliario
    Resultado liberarPersonal(bool esMesero, int id);                      // Deja libre a un mesero o domiciliario

    Resultado agregarPlato(const std::string& nombre, double precio);      // Agrega un plato al menú
    Resultado quitarUltimoPlato();                                         // Quita el último plato del menú
    int buscarPlato(const std::string& nombre);                            // Posición del plato en el menú (-1 si no existe)
    Resultado agregarMesa(int capacidad);                                  // Agrega una mesa

    std::string estadoJSON();                                              // Todo el estado del programa en formato JSON
};

#endif                    // Fin de la protección contra lecturas repetidas
