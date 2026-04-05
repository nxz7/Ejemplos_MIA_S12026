#include "reportes.h"
#include <cmath>
#include <fstream>

void reporte_disk(std::vector<disco> &discos, int disk_position, std::string &ruta) {
    // ABRIRI EL DISCO
    disco &uso = discos[disk_position];

    FILE *disk_file = fopen(uso.ruta.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR] NO SE PUDO ABRIR LA DATA DL DISK" << std::endl;
        return;
    }
//ABRIR EL MBR Y LEERLO
    MBR mbr;
    fseek(disk_file, 0, SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, disk_file);

    double size = static_cast<double>(mbr.mbr_tamano);

    // SI HAY EXTENDIDA BUSCAR PARA SABER SI HAY PARTICIONES LOGICAS y mostrarlas
    int EBR_position = -1;
    int final_extendida = -1;
    for (int i = 0; i < 4; ++i) {
        if (mbr.mbr_partition[i].part_type == 'e') {
            EBR_position = mbr.mbr_partition[i].part_start;
            final_extendida = mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s;
            break;
        }
    }

    // ordernar las particiones y la data
    std::vector<position> posiciones;
    for (int i = 0; i < 4; ++i) {
        if (mbr.mbr_partition[i].part_name[0] != '\0') {
            position partition_data;
            partition_data.inicio = mbr.mbr_partition[i].part_start;
            partition_data.fin    = mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s - 1;
            partition_data.nombre = mbr.mbr_partition[i].part_name;
            partition_data.tipo   = mbr.mbr_partition[i].part_type;  // 'p' o 'e'
            partition_data.size   = mbr.mbr_partition[i].part_s;
            posiciones.push_back(partition_data);
        }
    }
//ordenar las posiciones
    if (!posiciones.empty()) {
        std::sort(posiciones.begin(), posiciones.end());
    }

    // ARMAR EL DOT -------------HELPERS
    std::string graph_codigo;
//-ESPACIO LIBRE
    auto addFreeCell = [&](int espacio_libre) {
        if (espacio_libre <= 0) return;
        int porcentaje = static_cast<int>(std::round((espacio_libre * 100.0) / size));
        graph_codigo.append("<TD ROWSPAN='3' WIDTH='100' BGCOLOR='#ffeedd'>LIBRE<BR/>");
        graph_codigo.append(std::to_string(porcentaje));
        graph_codigo.append("% </TD>");
    };
//partition primaria/EXT -contenedores
    auto addPartitionCell = [&](const position &p) {
        if (p.tipo == 'p') {
            // PRIMARIA
            graph_codigo.append("<TD ROWSPAN='3' BGCOLOR='#909cc2' WIDTH='100'>PRIMARIA<BR/>");
            graph_codigo.append(p.nombre);
            graph_codigo.append("<br/>");
            int porcentaje = static_cast<int>(std::round((p.size * 100.0) / size));
            graph_codigo.append(std::to_string(porcentaje));
            graph_codigo.append("%  </TD>");
        } else {
            // EXTENDIDA solo se hace el contenedor porque lleva logica
            graph_codigo.append("<TD COLSPAN='50' BGCOLOR='#f58a07' WIDTH='100'>EXTENDIDA<BR/></TD>");
        }
    };

    // ------------------------- CONSTRUIR LA TABLA
    graph_codigo = "digraph mbr {"
                   "node [shape=plaintext]"
                   "struct1 [label= <<TABLE BORDER='2' CELLBORDER='1' CELLSPACING='0'>";

    graph_codigo.append("<TR>");
    graph_codigo.append("<TD ROWSPAN='3' BGCOLOR='#084887' HEIGHT='100'>MBR</TD>");

    if (posiciones.empty()) {
        // si todo libre
        addFreeCell(static_cast<int>(size));
    } else {
        for (int i = 0; i < static_cast<int>(posiciones.size()); ++i) {
            position &x = posiciones[i];

            // espacio libre
            if (i == 0) {
                int espacio_libre = x.inicio - EndMBR;
                addFreeCell(espacio_libre);
            }

            // particion
            addPartitionCell(x);

            // espacio libre entre esta y la siguiente
            if (i < static_cast<int>(posiciones.size()) - 1) {
                position &y = posiciones[i + 1];
                int espacio_libre = y.inicio - (x.fin + 1);
                addFreeCell(espacio_libre);
            } else {
                // ultima particion
                int espacio_libre = static_cast<int>(size) - (x.fin + 1);
                addFreeCell(espacio_libre);
            }
        }
    }

    graph_codigo.append("</TR>");

    // LOGICAS (van dentro de la extendida)
    if (EBR_position != -1 && final_extendida != -1) {
        graph_codigo.append("<TR>");

        EBR ebr;
        int actual_EBR_postion = EBR_position;

        while (true) {
            fseek(disk_file, actual_EBR_postion, SEEK_SET);
            fread(&ebr, sizeof(EBR), 1, disk_file);

            // logica
            graph_codigo.append("<TD BGCOLOR='#1b5ea1ff' HEIGHT='100'>EBR</TD>");

            int espacio_libre = 0;

            if (ebr.part_s > 0) {
                // part logica
                graph_codigo.append("<TD BGCOLOR='#f9ab55' WIDTH='100'>LOGICA<BR/>");
                graph_codigo.append(ebr.part_name);
                graph_codigo.append("<br/>");
                int porcentaje = static_cast<int>(std::floor((ebr.part_s * 100.0) / size));
                graph_codigo.append(std::to_string(porcentaje));
                graph_codigo.append("%  </TD>");

                int finLogica = ebr.part_start + ebr.part_s;
                if (ebr.part_next == -1) {
                    espacio_libre = final_extendida - finLogica;
                } else {
                    espacio_libre = ebr.part_next - finLogica;
                }
            } else {
                // EBR SIN LOGICA
                if (ebr.part_next == -1) {
                    espacio_libre = final_extendida - ebr.part_start;
                } else {
                    espacio_libre = ebr.part_next - ebr.part_start;
                }
            }

            addFreeCell(espacio_libre);
//SI NO HAY NEXT TERMINAR
            if (ebr.part_next == -1) {
                break; 
            }
            actual_EBR_postion = ebr.part_next;
        }

        graph_codigo.append("</TR>");
    }

    graph_codigo.append("</TABLE>>];}");
    fclose(disk_file);

    // HACER EL DOT
    std::ofstream outfile("disk_grafo.dot");
    outfile << graph_codigo << std::endl;
    outfile.close();
    std::string extension = "png";
    std::string::size_type pos = ruta.rfind('.');
    if (pos != std::string::npos && pos + 1 < ruta.size()) {
        extension = ruta.substr(pos + 1);
    }

    // HACER EL PNG
    std::string bash_command = "dot -T" + extension + " disk_grafo.dot -o'" + ruta + "'";
    system(bash_command.c_str());

    std::cout << "[SUCCESS] REPORTE DISK CREADO" << std::endl;
}
