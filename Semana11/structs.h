#ifndef STRUCTS
#define STRUCTS

#include <time.h>
#include <string>
#include <vector>
#include <regex>

// 0: inactiva, 1: activa

// PARTICION EN EL MBR - DATA A ALMACENAR y luego mostrar en el reporte
struct particion {
    char part_status;
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

//----------------------------------------------------

//********** EBR **********
struct EBR{
    char part_status;                 //0/1
    char part_fit;                    // best, worst, first
    int part_start;                   //Posicion inicial
    int part_s;                       //tamaño
    int part_next;                    //sigueinte
    char part_name[16];               //Nombre de la partición logica

};

//********** POSICIONES DE LAS PARTICIONES **********
//POSICIONES DE LAS PARTICIONES

struct position{
    // inicio = en donde empieza la particion
    // fin = en donde termina la particion

    int inicio;
    int fin;
    char tipo;
    std::string nombre;
    int size;

    bool operator<(const position& a) const
    {
        return inicio < a.inicio;
    }
};

// --------- ESPACIOS LIBRES particiones primarias y extendida
struct libre{
    int inicio;
    int size;

    bool operator<(const libre& a) const
    {
        return size < a.size;
    }
};

//espacio vacio - PARTICIONES LOGICAS
struct libreL{
    int inicioEBR;              //leerl el inicio
    int finLogica;              //Donde termina la particion logica
    int size;                   //Espacio Libre
    bool header;                //si es la inicial

    bool operator<(const libreL& a) const
    {
        return size < a.size;
    }
};



//-----------------------------MOUNT


//---------- MONTAR DISCO

struct montada{
    std::string id;                             //ID de a particion montada
    int posEBR = -1;                            //Si es logica es diferente a -1
    int posMBR = -1;                            //Si no es logica es diferente a -1
    std::string nombre;                         //Nombre de la particion
    int size;                                   //size de la particion en bytes
};

//MANEJA LOS DATOS DE LAS PARTICIONES MONTADAS
struct disco{
    std::string ruta;                           //Ruta del disco
    std::string nombre;                         //Nombre del disco
    int contador = 1;                           //Sirve para el ID de la montada
    std::vector<montada> particiones;           //Particiones del disco montadas
};

//SUPER
struct sbloque{
    int s_filesystem_type;              //EXT2 o EXT3
    int s_inodes_count;
    int s_blocks_count;
    int s_free_blocks_count;
    int s_free_inodes_count;
    time_t s_mtime;
    time_t s_umtime;
    int s_mnt_count;
    int s_magic;
    int s_inode_s;
    int s_block_s;
    int s_first_ino;
    int s_first_blo;
    int s_bm_inode_start;
    int s_bm_block_start;
    int s_inode_start;
    int s_block_start;
    int s_journal_start;                // solo EXT3
    int s_journal_count;                // entradas usadas
    int s_journal_max;                  // entradas disponibles
};

//INODES
struct inodo{
    int i_uid;
    int i_gid;
    int i_s;
    time_t i_atime;
    time_t i_ctime;
    time_t i_mtime;
    int i_block[15];
    char i_type;
    int i_perm;
};

//BLOQUES
struct content{
    char b_name[12];
    //numeor del inodo en el que esta el arhico/file
    int b_inodo;
};

struct bcarpetas{
    content b_content[4];
};

struct barchivos{
    char b_content[64];
};

struct bapuntadores{
    //posicion del otrro bloqie
    int b_pointers[16];
};

//registros, journaling - en caso de ext3
struct registro_journal{
    char j_command[16];
    char j_path[128];
    char j_content[64];
    time_t j_date;
    int j_owner;
};


//se usa para el login
struct usuario{
    std::string user;
    std::string pass;
    std::string disco;
    std::string grupo;
    std::string id_user;
    std::string id_grp;
};

// termina el mbr y ya se puede escribir la primera particion
const int EndMBR = sizeof(MBR) + 1;
const std::regex igual("=");

#endif
