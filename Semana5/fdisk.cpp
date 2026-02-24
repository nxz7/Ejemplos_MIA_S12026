#include "fdisk.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

void fdisk(std::vector<std::string> &parametros) {
    // VALIDACION

    int size = 0;                 // size
    std::string fit;              // ff/bf/wf
    std::string unidad;           // b/k/m
    std::string disk_path;             // path del disco
    std::string tipo;             // p/e/l
    std::string nombre;           // nombre de la partición
    char fit_char = 'f';          // f/b/w
    char tipo_char = 'p';         // p/e/l


// PARA QUE SEA LOWECASE

    auto to_lower_str = [](std::string &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    };

    //LEER LOS PARAMETROS Y ASIGNARLOS
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR] El parametro " << command_data
                      << " no tiene la forma correcta. Ejemplo: size=VALOR" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        to_lower_str(nombre_parametro);

        if (nombre_parametro == "size") {
            try {
                size = std::stoi(valor_parametro);
            } catch (...) {
                std::cout << "[ERROR] El size debe de ser un valor numerico." << std::endl;
                return;
            }
        } else if (nombre_parametro == "unit") {
            unidad = valor_parametro;
        } else if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else if (nombre_parametro == "type") {
            tipo = valor_parametro;
        } else if (nombre_parametro == "fit") {
            fit = valor_parametro;
        } else if (nombre_parametro == "name") {
            nombre = valor_parametro;
        } else {
            std::cout << "[ERROR] El parametro " << nombre_parametro
                      << " no es valido para fdisk (solo crear particiones)." << std::endl;
            return;
        }
    }

    // -------------- VALIDACIONES
    if (disk_path.empty() || nombre.empty()) {
        std::cout << "[ERROR]EL COMANDO FDISK NECESITA PATH Y NAME" << std::endl;
        return;
    }

    if (size <= 0) {
        std::cout << "[ERROR] EL SIZE DEBE SER MAYOR A 0." << std::endl;
        return;
    }

    // normalizar a minusculas fit, unidad, tipo
    to_lower_str(fit);
    to_lower_str(unidad);
    to_lower_str(tipo);

    //------------ DEFAULTS 
    // FIT
    if (fit.empty()) {
        // si no vienen nada el default el ff
        fit = "ff";
    }

    if (fit == "ff") {
        fit_char = 'f';
    } else if (fit == "bf") {
        fit_char = 'b';
    } else if (fit == "wf") {
        fit_char = 'w';
    } else {
        std::cout << "[ERROR]EL TIPO DE FIT NO ES VALIDO, DEBE SER: FF, BF, WF" << std::endl;
        return;
    }

    // TIPO
    if (tipo.empty()) {
        tipo = "p";
    }

    if (tipo == "p") {
        tipo_char = 'p';
    } else if (tipo == "e") {
        tipo_char = 'e';
    } else if (tipo == "l") {
        tipo_char = 'l';
    } else {
        std::cout << "[ERROR]EL TIPO  NO ES VALIDO, DEBE SER: p/e/l" << std::endl;
        return;
    }

    //unidad
    if (unidad.empty()) {
        unidad = "k";
    }

    int size_bytes = size;
    if (unidad == "m") {
        size_bytes = size * 1024 * 1024;
    } else if (unidad == "k") {
        size_bytes = size * 1024;
    } else if (unidad == "b") {
        size_bytes = size;  // ya viene en bytes
    } else {
        std::cout << "[ERROR] TIPO NO VALIDO, SOLO SE ACEPTA: B/K/M" << std::endl;
        return;
    }

    // ver que exista
    FILE *disk_file = fopen(disk_path.c_str(), "r+b");
    if (disk_file == nullptr) {
        std::cout << "[ERROR]el disco no existe_particion en ese path" << std::endl;
        return;
    }
    fclose(disk_file);

    particionar(size_bytes, tipo_char, disk_path, nombre, fit_char);
}




//---- funciones basicas de crear una particion

/*
Debe existir una ext para logicas

no debe haber otra con el mismo nombre

espacio libre para la particion

no pueden haver dos extendidas

slot para el MBR para primarias/extendida).
*/


void particionar(int &size, char &tipo, std::string &disk_path, std::string &nombre, char &fit) {
    //---------------------------------------------------------
    //1. abrir el disco y leer el MBR
    FILE *disk_file = fopen(disk_path.c_str(), "r+b");
    if (!disk_file) {
        std::cout << "[ERROR]NO SE PUDO ABRIR EL DISCO, revisar path" << std::endl;
        return;
    }
//mbr para poder leer la header del disco
    MBR mbr;
    // DEFAULT -1, es que no hay extendida
    int posicion = -1;
//posicionar al inicio
    fseek(disk_file, 0, SEEK_SET);
    // leer desde el inicio hasta el size del mbr --- leer la tabla de particiones
    fread(&mbr, sizeof(MBR), 1, disk_file);

    // ************** ELEGIR ESPACIO SEGUN EL FIT
    auto fitting_fit = [&](auto &espacios, int espacio_requerido) -> int {
        //espacios=vector de las sturcts libre, cada uno tiene un campo size
        //espacio_requerido=cuanto espacio necesita


        // SI EL ESPACIO DA MENOS UNO MARCA QUE NO HAY ESPACIO DISPONIBLE
        if (espacios.empty()) {
            return -1;
            // si no hay espacio
        }

        if (fit == 'f' || fit == 'b') {
            //ordena los bloques de menor a mayor y luego ya recorre y queda donde primero quepa
            if (fit == 'b') {
                std::sort(espacios.begin(), espacios.end()); 
            }
            //recorre en orden y devuelve el primer indice en donde el size cabe
            for (int i = 0; i < static_cast<int>(espacios.size()); ++i) {
                if (espacio_requerido <= espacios[i].size) {
                    return i;
                }
            }
            return -1;
        }

        if (fit == 'w') {
            //ordena de menor a mayor y elige el mayoir back()
            std::sort(espacios.begin(), espacios.end());
            if (espacio_requerido <= espacios.back().size) {
                return static_cast<int>(espacios.size()) - 1;
            }
            return -1;
        }

        return -1; 
    };

    // ------------------- PARTICIONES LOGICAS
    /*
    1. encontrar la extendida en el mbr
    2. dentro de la extendida recorrer los ebrs y encontrar espacios libres
    3. validar que no exista otra logica con el mismo
    4. elegir segun el fit
    5. crear - usando EBR
    */
    if (tipo == 'l') {
        int inicioExt;
        int finExt;
        EBR ebr;
        bool existe_particion = false;

        std::vector<libreL> espacios;
        bool header_visitada = false;
        bool keep_going = true;

        // 1. PRIMERO SE DEBE BUSCAR UNA EXTENDIDA EN EL MBR
        for (int i = 0; i < 4; i++) {
            if (mbr.mbr_partition[i].part_type == 'e') {
                posicion = i;
                break;
            }
        }

        if (posicion == -1) {
            std::cout << "[ERROR] NO EXISTE UNA EXTENDIDA - NO SE PUEDE CREAR LOGICA." << std::endl;
            fclose(disk_file);
            return;
        }

        // SEGUNDO LEER EL HEADER DE LA EXTENDIDA
        inicioExt = mbr.mbr_partition[posicion].part_start;
        finExt    = inicioExt + mbr.mbr_partition[posicion].part_s;

        //colocar en el inicio y leer el EBR (header) 
        // si part_next es -1 es que no hay mas ebr
        fseek(disk_file, inicioExt, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);

        // TERCERO RECORRER LOS EBR para ver el espacio libre y los nombre repetidos
        while (keep_going) {
            // revisa si ya existe una particion con ese nombre
            if (ebr.part_name == nombre) {
                existe_particion = true;
                break;
            }

            // ------- estructura libre
            libreL esp;
            // caso 1= da una vuelta y el header esta vacio ==0
            if (!header_visitada && ebr.part_s == 0) {
                esp.header = true;
                esp.inicioEBR = inicioExt; 
                esp.finLogica = ebr.part_start;

                if (ebr.part_next == -1) {
                    // no hay mas ebr, entonces el espacio libre es hasta el final de la extendida
                    esp.size = finExt - esp.finLogica;
                } else {
                    // hay mas ebr, entonces el espacio libre es hasta el siguiente ebr
                    esp.size = ebr.part_next - esp.finLogica;
                }
            } else {
                // caso 2 = ya no es el header o ya tiene datos. EL header ya tiene particion 
                esp.header = false;
                esp.inicioEBR = inicioExt;
                //ultimo byte ocupado por la logica actual
                esp.finLogica = ebr.part_start + ebr.part_s - 1;

                if (ebr.part_next == -1) {
                    //si no hay mas ebr, el espacio libre desde el fin de la logica hasta el fin de la extendida
                    esp.size = finExt - (esp.finLogica + 1);
                } else {
                    //si hay mas ebr, el espacio libre desde el fin de la logica hasta el siguiente ebr
                    esp.size = ebr.part_next - (esp.finLogica + 1);
                }
            }
            //si es mayor a 0 se añade a los espacios libres
            if (esp.size > 0) {
                espacios.push_back(esp);
            }

            header_visitada = true;

            if (ebr.part_next == -1) {
                // si ya no hay mas no se sigue
                keep_going = false;
            } else {
                // si aun hay mas sigue el loop. 
                // EL INICO ahora es el puntero next
                inicioExt = ebr.part_next;
                fseek(disk_file, inicioExt, SEEK_SET);
                fread(&ebr, sizeof(EBR), 1, disk_file);
            }
        }

        // ---------------------- VALIDACIONES LOGICAS
        if (existe_particion) {
            std::cout << "[ERROR] YA EXISTE UNA PARTICION LOGICA CON ESE NOMBRE" << std::endl;
            fclose(disk_file);
            return;
        }

        if (espacios.empty()) {
            std::cout << "[ERROR] LA PARTICIÓN EXTENDIDA NO TIENE ESPACIO LIBRE." << std::endl;
            fclose(disk_file);
            return;
        }

        //  ELEGIR EL ESPACIO SEGUN EL FIT
        int espacio_requerido = size + static_cast<int>(sizeof(EBR));
        //posEspacio es el indice en donde cabe
        int posEspacio = fitting_fit(espacios, espacio_requerido);

        if (posEspacio == -1) {
            std::cout << "[ERROR] NO HAY ESPACIO DISPONIBLE EN LA PARTICIÓN EXTENDIDA." << std::endl;
            fclose(disk_file);
            return;
        }

        //------------------- CREAR LA PARTICION LOGICA

        // caso 1: si el espacio libre es el header
        if (espacios[posEspacio].header) {
            // REESRRIBIR el header
            fseek(disk_file, espacios[posEspacio].inicioEBR, SEEK_SET);
            fread(&ebr, sizeof(EBR), 1, disk_file);

            std::strcpy(ebr.part_name, nombre.c_str());
            ebr.part_fit = fit;
            ebr.part_s = size;
            ebr.part_status = '0'; //aun no esta mounted

            fseek(disk_file, espacios[posEspacio].inicioEBR, SEEK_SET);
            fwrite(&ebr, sizeof(EBR), 1, disk_file);

            fclose(disk_file);
            std::cout << "[SUCCESS] PARTICION LOGICA CREADA CORRECTAMENTE" << std::endl;
        } else {
            // caso 2: si el espacio libre esta entre dos ebr o al final o no es la header
            //posEBR es el inicio del nuevo ebr, punto luego de la logica anterior
            int posEBR = espacios[posEspacio].finLogica + 1;
            int next;

            // Actualizar EBR padre para que el next ahora apunte al inicio del nuevo EBR
            fseek(disk_file, espacios[posEspacio].inicioEBR, SEEK_SET);
            fread(&ebr, sizeof(EBR), 1, disk_file);
            // NEXT ES A LO QUE ANTES APUNTABA
            next = ebr.part_next;
            ebr.part_next = posEBR;

            fseek(disk_file, espacios[posEspacio].inicioEBR, SEEK_SET);
            fwrite(&ebr, sizeof(EBR), 1, disk_file);

            // Crear nuevo EBR
            ebr.part_fit = fit;
            std::strcpy(ebr.part_name, nombre.c_str());
            ebr.part_status = '0';
            ebr.part_s = size;
            // LOS DATOS Empiezan luego del BER
            ebr.part_start = posEBR + sizeof(EBR);
            //el nuevo apunta a lo que antes apuntaba el padre
            ebr.part_next = next;
            //se escribe el nuevo EBR
            fseek(disk_file, posEBR, SEEK_SET);
            fwrite(&ebr, sizeof(EBR), 1, disk_file);

            fclose(disk_file);
            std::cout << "[SUCCESS] PARTICION LOGICA CREADA CORRECTAMENTE" << std::endl;
        }

        return; 
    }

    // PRIMARIA O EXTENDIDA 
    /*
    SOLO PUEDE EXISTIR UNA EXTENDIDA
    */
    if (tipo == 'p' || tipo == 'e') {
        bool extendedExist = false;
        bool existe_particion = false;
        // POSICIONES DE LAS PARTICIONES NO VACIAS
        std::vector<position> posiciones;
        std::vector<libre> espacios;
        int posEspacio = -1;

        // ------ 1. Buscar slot libre en el MBR y detectar si ya hay extendida
        for (int i = 0; i < 4; i++) {
            if (mbr.mbr_partition[i].part_name[0] == '\0') {
                //si encuentra una parte libre toma esa posicion
                posicion = i;
            }

            if (mbr.mbr_partition[i].part_type == 'e') {
                //revisar si hay una extendida
                extendedExist = true;
            }

            if (posicion != -1) {
                // si no hay entradas, las 4 estan usadas entonces para
                break;
            }
        }

        // validaciones, SOLO UNA EXTENDIDA y NO HAY ESPACIO
        if (extendedExist && tipo == 'e') {
            std::cout << "[ERROR] SOlo puede existir una particion extendida" << std::endl;
            fclose(disk_file);
            return;
        }

        if (posicion == -1) {
            std::cout << "[ERROR]No hay espacio, ya hay 4 particiones" << std::endl;
            fclose(disk_file);
            return;
        }

        // cargar las particiones existentes y verificar los nombres
        for (int i = 0; i < 4; i++) {
            if (mbr.mbr_partition[i].part_name[0] != '\0') {
                position pos;
                pos.inicio = mbr.mbr_partition[i].part_start;
                pos.fin = mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s - 1;
                pos.nombre = mbr.mbr_partition[i].part_name;
                //POSICIONES ALMACENA EL INICIO Y FIN DE CADA PARTICION
                posiciones.push_back(pos);

                if (nombre == mbr.mbr_partition[i].part_name) {
                    existe_particion = true;
                }
            }
        }

        if (existe_particion) {
            std::cout << "[ERROR] Ya existe una particion con ese nombre" << std::endl;
            fclose(disk_file);
            return;
        }

        // ordenar las posiciones si no esta vacio
        if (!posiciones.empty()) {
            std::sort(posiciones.begin(), posiciones.end());
        }

        // si no hay particiones - Constriuir los espacion libre
        // si noy todo el espacio desde endMBR hasta el final del disco esta libre
        if (posiciones.empty()) {
            libre esp;
            esp.inicio = EndMBR;
            esp.size = mbr.mbr_tamano - EndMBR;
            espacios.push_back(esp);
        } else {
            // si si hay particiones se debe buscar los espacios libres entre ellas
            for (int i = 0; i < static_cast<int>(posiciones.size()); i++) {
                // esp = stcruct libre
                libre esp;
                // x = posicion actual
                position &x = posiciones[i];
                //tamaño del espacio
                int espacio_libre = 0;
                /*
                CASOS:
                1. HAY MAS DE UNA Y ESTAMOS EN LA PRIMERA PARTICION
                2. SOLO HAY UNA PARTICION
                3. PARTICION INTERMEDIA
                4. ULTIMA PARTICION
                */
                if (i == 0 && i != (posiciones.size() - 1)) {
                    // HAY MAS DE UNA Y ESTAMOS EN LA PRIMERA PARTICION

                    // 1 espacio entre el final del mbr y el inicio de la primera particion
                    espacio_libre = x.inicio - EndMBR;
                    if (espacio_libre > 0) {
                        esp.inicio = EndMBR;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                    // 2 espacio entre la primera y la segunda particion )(
                    position &y = posiciones[i + 1];
                    // espacio justo despues de la primera particion
                    espacio_libre = y.inicio - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                } else if (i == 0 && i == (posiciones.size() - 1)) {
                    //2. SOLO HAY UNA PARTICION

                    //espacio entre el MBR y la particcion
                    espacio_libre = x.inicio - EndMBR;
                    if (espacio_libre > 0) {
                        esp.inicio = EndMBR;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                    // espacio entre esa particion y el final del disco
                    espacio_libre = mbr.mbr_tamano - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                } else if (i != (posiciones.size() - 1)) {
                    // 3. PARTICION INTERMEDIA

                    // espacio entre la particion actual y la siguiente
                    position &y = posiciones[i + 1];
                    espacio_libre = y.inicio - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                } else {
                    // 4. ULTIMA PARTICION
                    // espacio entre la ultima particion y el final del disco
                    espacio_libre = mbr.mbr_tamano - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                }
            }
        }

        if (espacios.empty()) {
            std::cout << "[ERROR] NO HAY ESPACIO EN EL DISCO" << std::endl;
            fclose(disk_file);
            return;
        }

        // elegir el espacio segun el fit
        posEspacio = fitting_fit(espacios, size);

        if (posEspacio == -1) {
            std::cout << "[ERROR] NO HAY ESPACIO EN EL DISCO PARA LA PARTICION" << std::endl;
            fclose(disk_file);
            return;
        }

        //----------------- CREAR LA PARTICION
        mbr.mbr_partition[posicion].part_fit   = fit;
        std::strcpy(mbr.mbr_partition[posicion].part_name, nombre.c_str());
        mbr.mbr_partition[posicion].part_status = '0'; // no montada
        mbr.mbr_partition[posicion].part_s      = size;
        mbr.mbr_partition[posicion].part_type   = tipo; // p/e

        //inicio del bloque libre elegido
        mbr.mbr_partition[posicion].part_start  = espacios[posEspacio].inicio;

        fseek(disk_file, 0, SEEK_SET);
        fwrite(&mbr, sizeof(MBR), 1, disk_file);

        // SI ES EXT SE DEBE CREAR EL EBR INICIAL
        // el ebr inidial es la header que luego se usa con las particiones logicas
        if (tipo == 'e') { 
            EBR ebr;
            //nombre vacio, part_next -1 (no hay logicas)
            std::strcpy(ebr.part_name, "");
            ebr.part_next  = -1;
            // inicio de la extendida + tamaño del ebr (donde inician los datos)
            ebr.part_start = espacios[posEspacio].inicio + sizeof(EBR) + 1;
            ebr.part_s     = 0;
            ebr.part_status = '0';

            fseek(disk_file, espacios[posEspacio].inicio, SEEK_SET);
            fwrite(&ebr, sizeof(EBR), 1, disk_file);
        }

        fclose(disk_file);
        std::cout << "[SUCCESS] LA PARTICION E/P CREADA CORRECTAMENTE" << std::endl;
    }
}
