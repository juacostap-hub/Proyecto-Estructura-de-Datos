#ifndef GRUPOPERSONAL_H   // Evita que este archivo se lea más de una vez al compilar
#define GRUPOPERSONAL_H   // Marca que este archivo ya fue leído

#include <string>         // Para usar texto (std::string)
#include "ColaCircular.h" // Es necesario porque el grupo usa una ColaCircular para los turnos


// Grupo de empleados que se turnan usando una COLA CIRCULAR.
// Se usa dos veces en el proyecto: un grupo de meseros y un grupo de domiciliarios.
// Cuando se necesita a alguien, se busca al primero libre en el turno; al asignarlo,
// pasa al final de la cola. Así el trabajo se reparte de forma justa entre todos.


class GrupoPersonal {
  private:
    std::string tipo;      // Nombre del cargo en singular: "Mesero" o "Domiciliario"
    std::string plural;    // Nombre del cargo en plural: "Meseros" o "Domiciliarios"
    ColaCircular cola;     // Cola circular con los turnos de los empleados
    int siguienteId;       // Número que se le dará al próximo empleado (para que cada uno tenga un id distinto)

  public:
    GrupoPersonal(const std::string& tipo, const std::string& plural, int capacidad);   // Constructor: crea un grupo vacío con su capacidad máxima

    bool agregar(const std::string& nombre);               // Agrega un empleado nuevo. Devuelve false si el grupo está lleno

    bool asignarSiguienteLibre(Empleado& asignado);        // Asigna al siguiente empleado libre según el turno. Devuelve false si todos están ocupados

    bool liberar(int id);                                  // Marca como libre al empleado con ese id. Devuelve false si no existe

    void mostrar() const;                                  // Muestra en pantalla el estado de todos los empleados

    int total() const;                                     // Cuántos empleados hay en el grupo
    int cantidadLibres() const;                            // Cuántos empleados están libres ahora
    const std::string& getTipo() const { return tipo; }        // Devuelve el cargo en singular
    const std::string& getPlural() const { return plural; }    // Devuelve el cargo en plural
};

#endif                    // Fin de la protección contra lecturas repetidas
