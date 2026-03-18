#include "reportes.h"
#include <cctype>
#include <fstream>

namespace {
    // limpiar el texto para que se pueda mostrar sin problemas
std::string escape_html(const std::string &text) {
    std::string escaped;
    escaped.reserve(text.size() + 16);

    for (char c : text) {
        switch (c) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&apos;"; break;
            case '\n': escaped += "<BR ALIGN=\"LEFT\"/>"; break;
            case '\r': break;
            default:
                if (std::isprint(static_cast<unsigned char>(c)) || c == '\t') {
                    escaped.push_back(c);
                } else {
                    escaped.push_back(' ');
                }
                break;
        }
    }
    return escaped;
}

std::string fixed_to_string(const char *data, size_t max_len) {
    size_t len = 0;
    while (len < max_len && data[len] != '\0') {
        ++len;
    }
    return std::string(data, len);
}
} 

void reporte_block(std::vector<disco> &discos, int posDisco, int posParticion, std::string &ruta) {
    disco &disc_uso = discos[posDisco];
    montada &part_uso = disc_uso.particiones[posParticion];

    FILE *disk_file = fopen(disc_uso.ruta.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR] no se encontro el disco" << std::endl;
        return;
    }

    int particion_inicio = 0;

    if (part_uso.posMBR != -1) {
        MBR mbr;
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        particion_inicio = mbr.mbr_partition[part_uso.posMBR].part_start;
    } else {
        EBR ebr;
        fseek(disk_file, part_uso.posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        particion_inicio = ebr.part_start;
    }

    sbloque sblock;
    fseek(disk_file, particion_inicio, SEEK_SET);
    fread(&sblock, sizeof(sbloque), 1, disk_file);

    std::vector<int> bloques_usados;
    std::vector<char> tipos;
    char bit_type = '0';

    for (int i = 0; i < sblock.s_blocks_count; ++i) {
        long pos_bm = sblock.s_bm_block_start + (i * sizeof(char));
        fseek(disk_file, pos_bm, SEEK_SET);
        fread(&bit_type, sizeof(char), 1, disk_file);

        if (bit_type == 'c' || bit_type == 'p' || bit_type == 'a') {
            bloques_usados.push_back(i);
            tipos.push_back(bit_type);
        }
    }

    std::string dot;
    dot += "digraph BlockReport {\n";
    dot += "  rankdir=LR;\n";
    dot += "  node [shape=plaintext, fontname=\"Helvetica\", fontsize=10];\n";
    dot += "  edge [color=\"#264653\", penwidth=1.4];\n";

    for (size_t i = 0; i < bloques_usados.size(); ++i) {
        int block_index = bloques_usados[i];
        long pos_block = sblock.s_block_start + (block_index * sblock.s_block_s);
        fseek(disk_file, pos_block, SEEK_SET);

        std::string node_id = "block" + std::to_string(block_index);
        std::string title;

        if (tipos[i] == 'c') {
            bcarpetas folder_block;
            fread(&folder_block, sizeof(bcarpetas), 1, disk_file);
            title = "Bloque Carpeta " + std::to_string(block_index);

            dot += "  " + node_id + " [label=<\n";
            dot += "    <TABLE BORDER=\"2\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\" BGCOLOR=\"#D8F3DC\" COLOR=\"#2D6A4F\">\n";
            dot += "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#95D5B2\"><B>" + title + "</B></TD></TR>\n";
            dot += "      <TR><TD><B>Nombre</B></TD><TD><B>Inodo</B></TD></TR>\n";

            for (int j = 0; j < 4; ++j) {
                std::string name = fixed_to_string(folder_block.b_content[j].b_name, sizeof(folder_block.b_content[j].b_name));
                if (name.empty()) {
                    name = "-";
                }
                dot += "      <TR><TD>" + escape_html(name) + "</TD><TD>" +
                       std::to_string(folder_block.b_content[j].b_inodo) + "</TD></TR>\n";
            }
            dot += "    </TABLE>\n";
            dot += "  >];\n";
        } else if (tipos[i] == 'p') {
            bapuntadores pointer_block;
            fread(&pointer_block, sizeof(bapuntadores), 1, disk_file);
            title = "Bloque Apuntador " + std::to_string(block_index);

            dot += "  " + node_id + " [label=<\n";
            dot += "    <TABLE BORDER=\"2\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\" BGCOLOR=\"#FAEDCD\" COLOR=\"#BC6C25\">\n";
            dot += "      <TR><TD BGCOLOR=\"#FFD166\"><B>" + title + "</B></TD></TR>\n";

            for (int j = 0; j < 16; ++j) {
                dot += "      <TR><TD>" + std::to_string(pointer_block.b_pointers[j]) + "</TD></TR>\n";
            }
            dot += "    </TABLE>\n";
            dot += "  >];\n";
        } else if (tipos[i] == 'a') {
            barchivos file_block;
            fread(&file_block, sizeof(barchivos), 1, disk_file);
            title = "Bloque Archivo " + std::to_string(block_index);

            std::string content = fixed_to_string(file_block.b_content, sizeof(file_block.b_content));
            if (content.empty()) {
                content = "(vacio)";
            }

            dot += "  " + node_id + " [label=<\n";
            dot += "    <TABLE BORDER=\"2\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\" BGCOLOR=\"#E0FBFC\" COLOR=\"#3D5A80\">\n";
            dot += "      <TR><TD BGCOLOR=\"#98C1D9\"><B>" + title + "</B></TD></TR>\n";
            dot += "      <TR><TD ALIGN=\"LEFT\">" + escape_html(content) + "</TD></TR>\n";
            dot += "    </TABLE>\n";
            dot += "  >];\n";
        }
    }

    for (size_t i = 0; i + 1 < bloques_usados.size(); ++i) {
        dot += "  block" + std::to_string(bloques_usados[i]) + " -> block" +
               std::to_string(bloques_usados[i + 1]) + ";\n";
    }

    dot += "}\n";
    fclose(disk_file);

    std::ofstream out("block.dot");
    out << dot;
    out.close();

    std::string extension = "png";
    std::string::size_type pos = ruta.rfind('.');
    if (pos != std::string::npos && pos + 1 < ruta.size()) {
        extension = ruta.substr(pos + 1);
    }

    std::string cmd = "dot -T" + extension + " block.dot -o\"" + ruta + "\"";
    system(cmd.c_str());

    std::cout << "[SUCCESS] REPORTE DE BLOQUES CREADO CORRECTAMENTE" << std::endl;
}
