#include "mount.h"

void mount(std::vector<std::string> &parametros, std::vector<disco> &discos) {
    // PARAMETROS PATH Y EL  NOMBRE DE LA PARTITION
    std::string disk_path;
    std::string nombre;

    // leer los parametros
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() != 2) {
            std::cout << "[ERROR] Parametro invalido: " << command_data
                      << " (debe ser clave=valor, ej: path=/disco.dk name=part1)" << std::endl;
            return;
        }

        std::string nombre_parametro   = output_data[0];
        std::string valor_parametro = output_data[1];

        //pasar a minusculas
        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else if (nombre_parametro == "name") {
            nombre = valor_parametro;
        } else {
            std::cout << "[ERROR] Parametro NO VALIDO - solo se aceptan path y name" << std::endl;
            return;
        }
    }

    // VALIDACION DE PARAMETROS OBLIGATORIOS
    if (disk_path.empty() || nombre.empty()) {
        std::cout << "[ERROR] mount requiere los parametros path y name." << std::endl;
        return;
    }

    // -------------- ABRIR DISCO Y LEER MBR
    FILE *disk_file = fopen(disk_path.c_str(), "r+b"); // leer y escribir
    if (!disk_file) {
        std::cout << "[ERROR] EL DISCO NO SE PUDO ABRIR - revisar path" << std::endl;
        return;
    }

    // MBR CONTIENE LOS DATOS DEL DISCO Y LAS PARTICIONES 
    MBR mbr;
    fseek(disk_file, 0, SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, disk_file);

    // valores default
    int posMBR   = -1;   // posicion en el MBR (INDICE 1-4)
    int posLogica = -1;  // posicion del EBR logica (si no es logica queda asi)
    int posEBR   = -1;   // recorrer el EBR, posicion
    int size     = 0;    // tamaño de la particion
    bool mount_ext = false;
    EBR ebr;

    // BUSCAR LA PARTICION EN EL MBR
    for (int i = 0; i < 4; ++i) {
        particion p = mbr.mbr_partition[i];
        // si el nombre coincide
        if (p.part_name == nombre) {
            if (p.part_type != 'e') {
                // si no es extendida, establecer los datos
                posMBR = i;
                size   = p.part_s;
                break;
            } else {
                // si se montan extendidas
                posLogica = p.part_start; // AQUI ESTA EL EBR
                size = 0;
                mount_ext = true;
                break;
            }
        }
    }

    // SI NO ESTA EN EXTENDIDAS NI MONTADAS ES LOGICA
    if (posMBR == -1 && !mount_ext) {
        // buscar la ext en donde estan las logicas

        //inicioEXT marca donde inicia la extendida
        int inicioExt = -1;
        for (int i = 0; i < 4; ++i) {
            if (mbr.mbr_partition[i].part_type == 'e') {
                inicioExt = mbr.mbr_partition[i].part_start;
                break;
            }
        }

        if (inicioExt == -1) {
            std::cout << "[ERROR] LA PARTICION NO EXISTE " << std::endl;
            fclose(disk_file);
            return;
        }

        // RECORRER LA LISTA DE EBR y encontrar la logica
        posEBR = inicioExt;
        fseek(disk_file, posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);


        //Recorrer la lista de EBR dentro de la extendida
        bool l_particion = false;
        while (true) {
            if (ebr.part_name == nombre) {
                //si encuentra el nombre
                l_particion = true;
                posLogica  = posEBR;      // ebr de la logica
                size       = ebr.part_s;  // size de la logica
                break;
            }
            //ya se busco en la lista de ebr y no hay mas
            if (ebr.part_next == -1) {
                break; 
            }
            // seguir al siguiente ebr
            posEBR = ebr.part_next;
            fseek(disk_file, posEBR, SEEK_SET);
            fread(&ebr, sizeof(EBR), 1, disk_file);
        }

        if (!l_particion) {
            std::cout << "[ERROR] LA PARTICION NO EXISTE" << std::endl;
            fclose(disk_file);
            return;
        }
    }

    //Registrar el montaje en la estructura discos

    //  posMBR != -1  - PARTICION P/E en  mbr.mbr_partition[posMBR]
    // posLogica != -1 => partición lógica con EBR en offset posLogica

    // --------- MONTAR EN EL VECTOR DE DISCOS

    //OBTENER EL NOMBRE DEL DISCO
    ///home/user/disco.dk nombre disco
    std::string discName = std::filesystem::path(disk_path).stem(); 

    // buscar si el disco ya esta en discos
    disco *diskEntry = nullptr;
    for (auto &d : discos) {
        if (d.nombre == discName) {
            diskEntry = &d;
            break;
        }
    }

    // si no esta se crea
    if (!diskEntry) {
        discos.emplace_back();
        disco &d = discos.back();
        d.nombre   = discName;
        d.ruta     = disk_path;
        d.contador = 0;
        diskEntry  = &d;
    }

    // verificar que la particion no este montada
    // recorre la liste de particones montadas
    for (const auto &p : diskEntry->particiones) {
        if (p.nombre == nombre) {
            std::cout << "[ERROR] LA PARTICION " << nombre << " YA ESTA MONTADA" << std::endl;
            fclose(disk_file);
            return;
        }
    }

    // ------------- CREAR EL MOUNT
    montada nueva;
    nueva.id     = "007" + std::to_string(diskEntry->contador) + discName;
    diskEntry->contador++;
    // si no es logica la posicion sera -1
    nueva.posEBR = posLogica;
    nueva.posMBR = posMBR;
    nueva.nombre = nombre;
    nueva.size   = size;

    // se inserta para que se recuerde que esta montada
    diskEntry->particiones.push_back(nueva);

    // ACTUALIZAR EL ESTADO EN EL DISCO status =1
    if (posLogica == -1) {
        // PRIMARIA EXTENDIDA ---> ACTUALIZAR MBR
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        mbr.mbr_partition[posMBR].part_status = '1'; 

        fseek(disk_file, 0, SEEK_SET);
        fwrite(&mbr, sizeof(MBR), 1, disk_file);

        // ACTUALIZAR EL SUPERBLOQEU
        sbloque bloque;
        fseek(disk_file, mbr.mbr_partition[posMBR].part_start, SEEK_SET);
        fread(&bloque, sizeof(sbloque), 1, disk_file);

        if (bloque.s_filesystem_type == 2) {
            //Incrementa las veces y pone la fecha
            bloque.s_mnt_count++;
            bloque.s_mtime = time(nullptr);
            fseek(disk_file, mbr.mbr_partition[posMBR].part_start, SEEK_SET);
            fwrite(&bloque, sizeof(sbloque), 1, disk_file);
        }
    } else {
        // LOGICA, actualizar el EBR
        fseek(disk_file, posLogica, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        ebr.part_status = '1';

        fseek(disk_file, posLogica, SEEK_SET);
        fwrite(&ebr, sizeof(EBR), 1, disk_file);

        // ACTUALIZAR EL SUPERBLOQUE
        sbloque bloque;
        fseek(disk_file, ebr.part_start, SEEK_SET);  // SE USA EL INICIO DEL SUPERBLOQUE
        fread(&bloque, sizeof(sbloque), 1, disk_file);

        if (bloque.s_filesystem_type == 2 ) {
            bloque.s_mnt_count++;
            bloque.s_mtime = time(nullptr);
            fseek(disk_file, ebr.part_start, SEEK_SET);
            fwrite(&bloque, sizeof(sbloque), 1, disk_file);
        }
    }

    fclose(disk_file);


    //------------------------------borrar
    // MOSTRAR EL LISTADO DE LAS PARTICIONES 
    std::cout << "[SUCCESS] PARTICION MONTADA" << std::endl;
    std::cout << ">>>>>>>>>>> ID: " << nueva.id << std::endl;

    std::cout << "\nLISTADO DE PARTICIONES MONTADAS\n\n";

    
    for (const auto &d : discos) {
        if (d.particiones.empty()) {
            continue;
        }

        std::cout << d.nombre << ":\n";
        for (const auto &p : d.particiones) {
            std::cout << "- " << p.nombre << std::endl;
        }
    }
}
