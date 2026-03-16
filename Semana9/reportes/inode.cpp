#include "reportes.h"
#include <fstream>
#include <cmath>
#include <ctime>

void reporte_inode(std::vector<disco> &discos, int posDisco, int posParticion, std::string &ruta) {
    // OBTENER EL DISCO Y LUEGO DE ESE SACAR LA PARTICION MONTADA 
    disco   &disc_uso = discos[posDisco];
    montada &part_uso = disc_uso.particiones[posParticion];

    // ABRIR EL DISCO
    FILE *disk_file = fopen(disc_uso.ruta.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR] No se encontro el archivo del disco." << std::endl;
        return;
    }

    // DETERMINAR EL INIDIO DE LA PARTICION
    int particion_inicio = 0;

    if (part_uso.posMBR != -1) {
        // CASO PRIMARIA/EXTENDIDA
        MBR mbr;
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        particion_inicio = mbr.mbr_partition[part_uso.posMBR].part_start;
    } else {
        // CASO PARTICION LOGICA
        EBR ebr;
        fseek(disk_file, part_uso.posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        particion_inicio = ebr.part_start;
    }

    // 1. ---------LEER EL SUPERBLOQUE DE LA PARTICION---------
    sbloque sblock;
    fseek(disk_file, particion_inicio, SEEK_SET);
    fread(&sblock, sizeof(sbloque), 1, disk_file);

    // LEER EL BITMAP DE INODOS Y CONSTRUIR LA LISTA DE INODOS USADOS
    std::vector<int> inodos_activos_1;
    char bitmap_status;

    for (int i = 0; i < sblock.s_inodes_count; ++i) {
        long pos_bm = sblock.s_bm_inode_start + i * sizeof(char);
        fseek(disk_file, pos_bm, SEEK_SET);
        fread(&bitmap_status, sizeof(char), 1, disk_file);

        if (bitmap_status == '1') {
            inodos_activos_1.push_back(i);
        }
    }

    // HACER EL DOT
    std::string dot;

    dot += "digraph Inodes {\n";
    dot += "  rankdir=LR;\n";
    dot += "  node [shape=plaintext, fontname=\"Helvetica\", fontsize=10, fontcolor=\"#226f54\"];\n";

    // --------------- FORMATEAR TIEMPOS
    auto formatTime = [](time_t t) -> std::string {
        if (t == 0) return "-";
        char buffer[32];
        std::tm *tm_info = std::gmtime(&t);
        if (!tm_info) return "-";
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        return std::string(buffer);
    };

    // TABLA POR CADA INODOS
    for (int idx : inodos_activos_1) {
        inodo linodo;
        long pos_inodo = sblock.s_inode_start + idx * sizeof(inodo);
        fseek(disk_file, pos_inodo, SEEK_SET);
        fread(&linodo, sizeof(inodo), 1, disk_file);

        std::string nodeName = "inode" + std::to_string(idx);
        std::string titulo   = "Inodo " + std::to_string(idx);

        dot += "  " + nodeName + " [label=<\n";
        dot += "    <TABLE BORDER=\"2\" CELLBORDER=\"1\" CELLSPACING=\"0\" "
               "CELLPADDING=\"4\" BGCOLOR=\"#f4f0bb\" COLOR=\"#87c38f\">\n";
        dot += "      <TR><TD COLSPAN=\"2\"><B>" + titulo + "</B></TD></TR>\n";

        auto addRow = [&](const std::string &campo, const std::string &valor) {
            dot += "      <TR><TD ALIGN=\"left\">" + campo + "</TD>"
                   "<TD>" + valor + "</TD></TR>\n";
        };

        addRow("id owner:",           std::to_string(linodo.i_uid));
        addRow("id grupo:",           std::to_string(linodo.i_gid));
        addRow("tamaño archivo:",     std::to_string(linodo.i_s));
        addRow("última lectura:",     formatTime(linodo.i_atime));
        addRow("fecha de creación:",  formatTime(linodo.i_ctime));
        addRow("última modificación:",formatTime(linodo.i_mtime));

        // APUNTADORES A BLOQUES
        for (int j = 0; j < 15; ++j) {
            addRow("Bloque " + std::to_string(j) + ":",
                   std::to_string(linodo.i_block[j]));
        }

        addRow("tipo de inodo:", std::string(1, linodo.i_type));
        addRow("permisos:",      std::to_string(linodo.i_perm));

        dot += "    </TABLE>\n";
        dot += "  >];\n";
    }

    // ---------- FLECHAS, ENLAZAR INODOS ----------
    if (inodos_activos_1.size() > 1) {
        dot += "  ";
        for (size_t i = 0; i < inodos_activos_1.size(); ++i) {
            dot += "inode" + std::to_string(inodos_activos_1[i]);
            if (i + 1 < inodos_activos_1.size()) {
                dot += " -> ";
            }
        }
        dot += ";\n";
    }

    dot += "}\n";

    fclose(disk_file);

    // GUARDAR
    std::ofstream outfile("inodes.dot");
    outfile << dot;
    outfile.close();

    // --HACER EL PNG
    std::string extension = "png";
    std::string::size_type pos = ruta.rfind('.');
    if (pos != std::string::npos && pos + 1 < ruta.size()) {
        extension = ruta.substr(pos + 1);
    }

    std::string cmd = "dot -T" + extension + " inodes.dot -o\"" + ruta + "\"";
    system(cmd.c_str());

    std::cout << "[SUCCESS] REPORTE DE INODO CREADO" << std::endl;
}
