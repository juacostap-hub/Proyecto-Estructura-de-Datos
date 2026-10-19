#pragma once            // Evita que este archivo se lea más de una vez al compilar
#include <iostream>     // Para escribir en pantalla (std::cout)
#include <string>       // Para usar texto (std::string) y std::to_string
#include <cstddef>      // Para la constante NULL (puntero que no apunta a nada)


// Tipos de tarjeta de fidelización (mientras más alto el número, más prioridad tiene).
const int SIN_TARJETA         = 0;    // Prioridad 0: Sin tarjeta de fidelización
const int CLIENTE_FRECUENTE   = 1;    // Prioridad 1: Cliente frecuente del restaurante
const int AMIGO_HOST          = 2;    // Prioridad 2: Amigo del gerente del restaurante
const int PRESTIGIO           = 3;    // Prioridad 3: Celebridad, políticos, influencers, prensa
const int VIP_POR_COMUN       = 2;    // Cantidad de clientes VIP que se atienden seguidos antes de llamar a un cliente de la fila común (2 VIP por 1 comun)
                                      // "const" significa que el valor no cambia mientras corre el programa (constante)


class Cliente{    // Un cliente que llega al restaurante y espera una mesa
  public:

    Cliente(){                        // Constructor vacío: Crea un cliente en blanco
      nombre = "";                    // Sin nombre
      personas = 0;                   // Sin personas
      tarjeta = SIN_TARJETA;          // Sin tarjeta
      turno = 0;                      // Sin turno asignado
    };

    Cliente(std::string _nombre, int _personas, int _tarjeta, int _turno){      // Constructor con datos: crea un cliente completo.
      nombre = _nombre;                                                         // Copia el nombre recibido
      personas = _personas;                                                     // Copia la cantidad de personas
      tarjeta = _tarjeta;                                                       // Copia el nivel de la tarjeta
      turno = _turno;                                                           // Copia el turno de llegada
    };

    int prioridad(){        // La prioridad del cliente es el nivel de su tarjeta
      return tarjeta;       // El nivel de la tarjeta es 0, 1, 2 o 3
    };

    bool esvip(){                         // Dice si el cliente tiene tarjeta (TRUE) o no (FALSE)
      return tarjeta != SIN_TARJETA;      // El nivel de la tarjeta será 0
    };

    std::string nombre_tarjeta(){                                          // Devuelve como texto el nombre del nivel de la tarjeta
      if(tarjeta==CLIENTE_FRECUENTE) return "Cliente frecuente";
      if(tarjeta==AMIGO_HOST) return "Amigo del gerente";
      if(tarjeta==PRESTIGIO) return "Persona con prestigio";
      return "Sin tarjeta";                                               // Si no tiene tarjeta (ninguno de los anteriores)
    };

    std::string convertir(){
      return "Turno " + std::to_string(turno) + " | " + nombre + " | " + std::to_string(personas) + " persona(s) | " + nombre_tarjeta();
    };

  public:
    std::string nombre;               // Nombre del cliente
    int personas;                     // Cantidad de personas del grupo
    int tarjeta;                      // Tipo de tarjeta de fidelización
    int turno;                        // Orden de llegada al restaurante
};


class NodoCola{     // Nodo que guarda un cliente y apunta al siguiente de la fila
  public:

    NodoCola(Cliente _dato){          // Crea un nodo con el cliente recibido
      dato = _dato;                   // Guarda una copia del cliente
      sig = NULL;                     // Al principio no apunta a ningún otro nodo
    };

  public:
    Cliente dato;                     // El cliente guardado en este nodo
    NodoCola * sig;                   // Puntero al nodo que viene después (NULL si es el último)
};
