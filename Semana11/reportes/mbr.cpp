#include "reportes.h"

void reporte_mbr(std::vector<disco> &discos, int posDisco, std::string &ruta){
    std::string graph_codigo;
    disco &actual_disk_data = discos[posDisco];
    FILE *disk_file;
    MBR mbr;
    int position_extendida;
    EBR ebr;
    std::string bash_command;

    disk_file = fopen(actual_disk_data.ruta.c_str(), "r+b");
    if(disk_file == NULL){
        std::cout << "[ERROR!] NO ESTA EL DISCO" << std::endl;
        return;
    }

    fseek(disk_file, 0,SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, disk_file);

    graph_codigo = "digraph mbr {node [shape=plaintext] struct1 [label= <<TABLE BORDER='2' CELLBORDER='0' CELLSPACING='0'>";
    
    graph_codigo.append("<TR>");
    graph_codigo.append("<TD BGCOLOR='#caffbf' WIDTH='300'> MBR</TD>");
    graph_codigo.append("<TD WIDTH='300' BGCOLOR='#caffbf'></TD>");
    graph_codigo.append("</TR>");

    graph_codigo.append("<TR>");
    graph_codigo.append("<TD>SIZE</TD>");
    graph_codigo.append("<TD>");
    graph_codigo.append(std::to_string(mbr.mbr_tamano));
    graph_codigo.append("</TD>");
    graph_codigo.append("</TR>");       

    graph_codigo.append("<TR>");
    graph_codigo.append("<TD>Fit</TD>");
    graph_codigo.append("<TD>");
    graph_codigo.push_back(mbr.dsk_fit);
    graph_codigo.append("</TD>");
    graph_codigo.append("</TR>");

    graph_codigo.append("<TR>");
    graph_codigo.append("<TD>Disk Signature</TD>");
    graph_codigo.append("<TD>");
    graph_codigo.append(std::to_string(mbr.mbr_dsk_signature));
    graph_codigo.append("</TD>");
    graph_codigo.append("</TR>");

    graph_codigo.append("<TR>");
    graph_codigo.append("<TD>Fecha Creacion</TD>");
    graph_codigo.append("<TD>");
    graph_codigo.append(asctime(gmtime(&mbr.mbr_fecha_creacion)));
    graph_codigo.append("</TD>");
    graph_codigo.append("</TR>");   

    for(int i = 0; i < 4; i++){
        if(mbr.mbr_partition[i].part_name[0] != '\0'){
            if(mbr.mbr_partition[i].part_type == 'p'){
                graph_codigo.append("<TR>");
                graph_codigo.append("<TD BGCOLOR='#f0f3bd' WIDTH='300'>PARTICION</TD>");
                graph_codigo.append("<TD WIDTH='300' BGCOLOR='#f0f3bd'></TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Status</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.push_back(mbr.mbr_partition[i].part_status);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Tipo</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.push_back(mbr.mbr_partition[i].part_type);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Fit</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.push_back(mbr.mbr_partition[i].part_fit);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Inicio</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.append(std::to_string(mbr.mbr_partition[i].part_start));
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>SIZE</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.append(std::to_string(mbr.mbr_partition[i].part_s));
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Nombre</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.append(mbr.mbr_partition[i].part_name);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");
            }

            if(mbr.mbr_partition[i].part_type == 'e'){
                graph_codigo.append("<TR>");
                graph_codigo.append("<TD BGCOLOR='#d1ac00' WIDTH='300'>PARTICION</TD>");
                graph_codigo.append("<TD WIDTH='300' BGCOLOR='#d1ac00'></TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Status</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.push_back(mbr.mbr_partition[i].part_status);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Tipo</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.push_back(mbr.mbr_partition[i].part_type);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Fit</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.push_back(mbr.mbr_partition[i].part_fit);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Inicio</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.append(std::to_string(mbr.mbr_partition[i].part_start));
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>SIZE</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.append(std::to_string(mbr.mbr_partition[i].part_s));
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                graph_codigo.append("<TR>");
                graph_codigo.append("<TD>Nombre</TD>");
                graph_codigo.append("<TD>");
                graph_codigo.append(mbr.mbr_partition[i].part_name);
                graph_codigo.append("</TD>");
                graph_codigo.append("</TR>");

                position_extendida = mbr.mbr_partition[i].part_start;
                while(true){
                    fseek(disk_file, position_extendida, SEEK_SET);
                    fread(&ebr, sizeof(EBR), 1, disk_file);

                    if(ebr.part_name[0] != '\0'){
                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD BGCOLOR='#bdb2ff' WIDTH='300'>EBR</TD>");
                        graph_codigo.append("<TD WIDTH='300' BGCOLOR='#bdb2ff'></TD>");
                        graph_codigo.append("</TR>");

                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD>Status</TD>");
                        graph_codigo.append("<TD>");
                        graph_codigo.push_back(ebr.part_status);
                        graph_codigo.append("</TD>");
                        graph_codigo.append("</TR>");

                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD>Fit</TD>");
                        graph_codigo.append("<TD>");
                        graph_codigo.push_back(ebr.part_fit);
                        graph_codigo.append("</TD>");
                        graph_codigo.append("</TR>");

                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD>Inicio</TD>");
                        graph_codigo.append("<TD>");
                        graph_codigo.append(std::to_string(ebr.part_start));
                        graph_codigo.append("</TD>");
                        graph_codigo.append("</TR>");

                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD>SIZE</TD>");
                        graph_codigo.append("<TD>");
                        graph_codigo.append(std::to_string(ebr.part_s));
                        graph_codigo.append("</TD>");
                        graph_codigo.append("</TR>");

                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD>Next</TD>");
                        graph_codigo.append("<TD>");
                        graph_codigo.append(std::to_string(ebr.part_next));
                        graph_codigo.append("</TD>");
                        graph_codigo.append("</TR>");

                        graph_codigo.append("<TR>");
                        graph_codigo.append("<TD>Nombre</TD>");
                        graph_codigo.append("<TD>");
                        graph_codigo.append(ebr.part_name);
                        graph_codigo.append("</TD>");
                        graph_codigo.append("</TR>");
                    }

                    if(ebr.part_next == -1){
                        break;
                    }else{
                        position_extendida = ebr.part_next;
                    }
                }
            }
        }
    }
    graph_codigo.append("</TABLE>>];}");
    fclose(disk_file);
    std::ofstream outfile ("mbr_dot.dot");
    outfile << graph_codigo << std::endl;
    outfile.close();

    std::string::size_type pos = ruta.rfind('.', ruta.length());
    std::string extension = ruta.substr(pos + 1, ruta.length() - 1);

    bash_command = "dot -T";
    bash_command.append(extension);
    bash_command.append(" mbr_dot.dot -o");
    bash_command.append("'");
    bash_command.append(ruta);
    bash_command.append("'");

    system(bash_command.c_str());
    std::cout << "[sUCCESS] REPORTE MBR CREADO  " << std::endl;
}
