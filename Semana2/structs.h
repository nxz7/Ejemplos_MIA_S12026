#ifndef STRUCTS
#define STRUCTS

#include <ctime>
#include <regex>

// PARTICION EN EL MBR - DATA A ALMACENAR y luego mostrar en el reporte
struct particion {
    char part_status;      // 0: inactiva, 1: activa
    char part_type;        // P: primaria, E: extendida, L: logica
    char part_fit;         // best, worst, first
    int part_start;        // Inicio en bytes
    int part_s;            // SIZE
    char part_name[16];    // Nombre
};

// MBR DEL DISCO
struct MBR {
    int mbr_tamano;
    time_t mbr_fecha_creacion;
    int mbr_dsk_signature;
    char dsk_fit;
    particion mbr_partition[4];
};

// --------- SEPARAR LOS VALORES - mas facil
const std::regex igual("=");

#endif
