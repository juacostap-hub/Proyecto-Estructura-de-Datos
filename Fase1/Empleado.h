#ifndef EMPLEADO_H        // Evita que este archivo se lea más de una vez al compilar
#define EMPLEADO_H        // Marca que este archivo ya fue leído

#include <string>         // Para usar texto (std::string)


// Un empleado de la cafetería. Sirve tanto para los meseros como para los domiciliarios.
struct Empleado {         // "struct" es como una clase, pero todo es público por defecto
    int id;               // Número único del empleado
    std::string nombre;   // Nombre del empleado
    bool ocupado;         // true = atendiendo o entregando, false = libre
    int servicios;        // Cuántas veces ha sido asignado (atenciones o entregas)

    Empleado(int id = 0, const std::string& nombre = "")          // Constructor: crea un empleado (si no se dan datos, queda en blanco)
        : id(id), nombre(nombre), ocupado(false), servicios(0) {} // Copia el id y el nombre; empieza libre y con 0 servicios
};

#endif                    // Fin de la protección contra lecturas repetidas
