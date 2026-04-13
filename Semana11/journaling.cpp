#include "journaling.h"

#include <cstdio>
#include <cstring>
#include <ctime>

void journaling(std::vector<std::string> &parametros, std::vector<disco> &discos) {
    std::string id;

    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &parametro = parametros[i];
//PARAMETROS COMO SIEMPRE
        std::vector<std::string> output_data(
            std::sregex_token_iterator(parametro.begin(), parametro.end(), igual, -1),
            std::sregex_token_iterator()
        );
//SI NO CUMPLKE CON EL FORMATO
        if (output_data.size() != 2) {
            std::cout << "[ERROR] PARAMETRO NO VALIDO " << parametro
                      << " formato esperado: id=0070Disco1" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];

        //ESTANDARIZAR
        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (nombre_parametro == "id") {
            id = output_data[1];
        } else {
            std::cout << "[ERROR] JOURNALING SOLO ACEPTA EL PARAMETRO ID" << std::endl;
            return;
        }
    }
//FIJO DEBE IR UN ID
    if (id.empty()) {
        std::cout << "[ERROR] JOURNALING REQUIERE EL PARAMETRO ID" << std::endl;
        return;
    }


    //extraer el nombre del disco en base al id
    size_t position_digits = 0;
    while (position_digits < id.size() && std::isdigit(static_cast<unsigned char>(id[position_digits]))) {
        ++position_digits;
    }

    //quita la letra para solo dejar el disco
    //id=0070Disco1 -> position_digits=4 -> diskName=Disco1
    std::string diskName = id.substr(position_digits);

// ----BUSCAR DISCO
    // aca es dpnde ya se busca el disco
    int posDisco = -1;
    for (int i = 0; i < static_cast<int>(discos.size()); ++i) {
        //se busca en el vector de discos montados para ver que si exista 
        //discos[i].nombre es el nombre del disco montado, diskName es el nombre extraido del id
        //se guarda la posicion
        if (discos[i].nombre == diskName) {
            posDisco = i;
            break;
        }
    }


    // como lo trabajamos siempre si no hay se usa -1
    if (posDisco == -1) {
        std::cout << "[ERROR] EL DISCO NO ESTA MONTADO" << std::endl;
        return;
    }

    // ---- BUSCAR LA PARTICION
    disco &mounted_disk = discos[posDisco];
    int posParticion = -1;

    for (int i = 0; i < static_cast<int>(mounted_disk.particiones.size()); ++i) {
        if (mounted_disk.particiones[i].id == id) {
            posParticion = i;
            break;
        }
    }

    if (posParticion == -1) {
        std::cout << "[ERROR] LA PARTICION NO EXISTE, REVISAR ID" << std::endl;
        return;
    }

    montada &particion_montada = mounted_disk.particiones[posParticion];

    //ABRE EL DISCO
    FILE *disk_file = fopen(mounted_disk.ruta.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR] NO SE PUDO ABRIR EL DISCO" << std::endl;
        return;
    }

    // 1. BUSCA EL INICIO DE LA PARTICION
    // MIRA SI ES LOGICA O PRIMARIA PARA SABER SI LEER EL MBR O EBR
    int particion_inicio = 0;
    if (particion_montada.posMBR != -1) {
        //mueve los punteros al inicio
        MBR mbr;
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        particion_inicio = mbr.mbr_partition[particion_montada.posMBR].part_start;
    } else {
        EBR ebr;
        fseek(disk_file, particion_montada.posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        particion_inicio = ebr.part_start;
    }

    // lee la metadata de la particion
    sbloque sblock;
    fseek(disk_file, particion_inicio, SEEK_SET);
    fread(&sblock, sizeof(sbloque), 1, disk_file);

    //verifica que sea ext3 y que tenga journaling
    // s_filesystem_type == 3 indica que es ext3, s_journal_start >= 0 y s_journal_max > 0 indican que tiene journaling
    if (sblock.s_filesystem_type != 3 || sblock.s_journal_start < 0 || sblock.s_journal_max <= 0) {
        std::cout << "[ERROR] LA PARTICION NO ESTA FORMATEADA EN EXT3" << std::endl;
        fclose(disk_file);
        return;
    }

    //mostrar

    std::cout << "----- JOURNALING (" << id << ") -----" << std::endl;

    // si no hay data no muestra nada
    if (sblock.s_journal_count <= 0) {
        std::cout << "[INFO] SIN REGISTROS" << std::endl;
        fclose(disk_file);
        return;
    }

    // limitar cuantos registros leer
    // toma el menor de la cantidad de registros y la capacidad maxima
    int limit = std::min(sblock.s_journal_count, sblock.s_journal_max);

    for (int i = 0; i < limit; ++i) {
        //aca se lee cada registro
        registro_journal registro{};

        //calcula que tanto avanza 
        //i * sizeof(registro_journal)
        int offset = sblock.s_journal_start + (i * static_cast<int>(sizeof(registro_journal)));
        fseek(disk_file, offset, SEEK_SET);
        fread(&registro, sizeof(registro_journal), 1, disk_file);

        // si no tiene nada valido
        if (registro.j_command[0] == '\0') {
            continue;
        }

        //sacar la fecha y formatear
        char fecha_buffer[32] = "N/A";
        std::tm *tm_info = std::localtime(&registro.j_date);
        if (tm_info != nullptr) {
            std::strftime(fecha_buffer, sizeof(fecha_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        }

        //mostrar
        std::cout << "#" << i
                  << " cmd=" << registro.j_command
                  << " path=" << registro.j_path
                  << " content=" << registro.j_content
                  << " date=" << fecha_buffer
                  << " owner=" << registro.j_owner << std::endl;
    }

    fclose(disk_file);
}
