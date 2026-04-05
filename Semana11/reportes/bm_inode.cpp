#include "reportes.h"
#include <fstream>

void reporte_bm_inode(std::vector<disco> &discos, int posDisco, int posParticion, std::string &ruta) {
    disco   &disc_uso = discos[posDisco];
    montada &part_uso = disc_uso.particiones[posParticion];

    // ABRIR EL ARCHIVO FISICO DEL DISCO
    FILE *disk_file = fopen(disc_uso.ruta.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR] No se encontró el archivo del disco." << std::endl;
        return;
    }

    // EL INICIO DE LA PARTICION)
    int particion_inicio = 0;

    if (part_uso.posMBR != -1) {
        // PRIMARIA O EXTENDIDA SE LEE EL MBR
        MBR mbr;
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        particion_inicio = mbr.mbr_partition[part_uso.posMBR].part_start;
    } else {
        // LOGICA SE LEE EL EBR
        EBR ebr;
        fseek(disk_file, part_uso.posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        particion_inicio = ebr.part_start;
    }

    // leer el sb y la particion
    sbloque sblock;
    fseek(disk_file, particion_inicio, SEEK_SET);
    fread(&sblock, sizeof(sbloque), 1, disk_file);

    // construit el texto
    std::string texto_bitmap;
    texto_bitmap.reserve(sblock.s_inodes_count * 2); // 0/1 por inodo

    char bit;
    int bits_en_linea = 0;

    for (int i = 0; i < sblock.s_inodes_count; ++i) {
        long pos_bit = sblock.s_bm_inode_start + i * sizeof(char);
        fseek(disk_file, pos_bit, SEEK_SET);
        fread(&bit, sizeof(char), 1, disk_file);

        //1 = usado 0 = libre
        texto_bitmap.push_back(bit == '1' ? '1' : '0');
        texto_bitmap.push_back(' ');

        bits_en_linea++;
        if (bits_en_linea == 20) {
            texto_bitmap.push_back('\n');
            bits_en_linea = 0;
        }
    }

    fclose(disk_file);

    // guardar el bitmapo en el archivo de salida
    std::ofstream outfile(ruta);
    outfile << texto_bitmap << std::endl;
    outfile.close();

    std::cout << "[SUCCESS] Reporte bm_inode creado correctamente." << std::endl;
}
