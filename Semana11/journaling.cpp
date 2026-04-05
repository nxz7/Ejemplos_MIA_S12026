#include "journaling.h"

#include <cstdio>
#include <cstring>
#include <ctime>

void journaling(std::vector<std::string> &parametros, std::vector<disco> &discos) {
    std::string id;

    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &parametro = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(parametro.begin(), parametro.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() != 2) {
            std::cout << "[ERROR] PARAMETRO NO VALIDO " << parametro
                      << " formato esperado: id=0070Disco1" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (nombre_parametro == "id") {
            id = output_data[1];
        } else {
            std::cout << "[ERROR] JOURNALING SOLO ACEPTA EL PARAMETRO ID" << std::endl;
            return;
        }
    }

    if (id.empty()) {
        std::cout << "[ERROR] JOURNALING REQUIERE EL PARAMETRO ID" << std::endl;
        return;
    }

    size_t position_digits = 0;
    while (position_digits < id.size() && std::isdigit(static_cast<unsigned char>(id[position_digits]))) {
        ++position_digits;
    }
    std::string diskName = id.substr(position_digits);

    int posDisco = -1;
    for (int i = 0; i < static_cast<int>(discos.size()); ++i) {
        if (discos[i].nombre == diskName) {
            posDisco = i;
            break;
        }
    }

    if (posDisco == -1) {
        std::cout << "[ERROR] EL DISCO NO ESTA MONTADO" << std::endl;
        return;
    }

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
    FILE *disk_file = fopen(mounted_disk.ruta.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR] NO SE PUDO ABRIR EL DISCO" << std::endl;
        return;
    }

    int particion_inicio = 0;
    if (particion_montada.posMBR != -1) {
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

    sbloque sblock;
    fseek(disk_file, particion_inicio, SEEK_SET);
    fread(&sblock, sizeof(sbloque), 1, disk_file);

    if (sblock.s_filesystem_type != 3 || sblock.s_journal_start < 0 || sblock.s_journal_max <= 0) {
        std::cout << "[ERROR] LA PARTICION NO ESTA FORMATEADA EN EXT3" << std::endl;
        fclose(disk_file);
        return;
    }

    std::cout << "----- JOURNALING (" << id << ") -----" << std::endl;

    if (sblock.s_journal_count <= 0) {
        std::cout << "[INFO] SIN REGISTROS" << std::endl;
        fclose(disk_file);
        return;
    }

    int limit = std::min(sblock.s_journal_count, sblock.s_journal_max);

    for (int i = 0; i < limit; ++i) {
        registro_journal registro{};
        int offset = sblock.s_journal_start + (i * static_cast<int>(sizeof(registro_journal)));
        fseek(disk_file, offset, SEEK_SET);
        fread(&registro, sizeof(registro_journal), 1, disk_file);

        if (registro.j_command[0] == '\0') {
            continue;
        }

        char fecha_buffer[32] = "N/A";
        std::tm *tm_info = std::localtime(&registro.j_date);
        if (tm_info != nullptr) {
            std::strftime(fecha_buffer, sizeof(fecha_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        }

        std::cout << "#" << i
                  << " cmd=" << registro.j_command
                  << " path=" << registro.j_path
                  << " content=" << registro.j_content
                  << " date=" << fecha_buffer
                  << " owner=" << registro.j_owner << std::endl;
    }

    fclose(disk_file);
}
