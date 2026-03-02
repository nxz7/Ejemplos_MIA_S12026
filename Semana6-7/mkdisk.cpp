#include "mkdisk.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <ctime>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

void mkdisk(std::vector<std::string> &parametros) {
    // ------------- DATA DEL DISCO
    FILE *disk_file = nullptr;
    char vacio = '\0';
    int disk_size = 0;            // TAMAÑO
    std::string fit;              // ff/bf/wf
    char fit_char = '0';          // f/b/w
    std::string disk_unit;        // k/m
    std::string disk_path;        // PATH
    MBR mbr;

    // PASAR A MINUSCULA TODO
    auto to_lower_str = [](std::string &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    };

    // leer los parametros y establecer el valor de cada variable size, fit, unit, path
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR!] El parametro " << command_data
                      << " no cuenta con la forma correcta. EJEMPLO: size=VALOR" << std::endl;
            return;
        }

        // nombre_parametro = fit, size, path, unit
        // valor_parametro = valor asignado al parametro
        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        to_lower_str(nombre_parametro);

        if (nombre_parametro == "size") {
            //probar pasarlo a entero si no error
            try {
                disk_size = std::stoi(valor_parametro);
            } catch (...) {
                std::cout << "[ERROR!] El parametro size debe ser un entero" << std::endl;
                return;
            }
        } else if (nombre_parametro == "fit") {
            to_lower_str(valor_parametro);
            fit = valor_parametro;
        } else if (nombre_parametro == "unit") {
            to_lower_str(valor_parametro);
            disk_unit = valor_parametro;
        } else if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else {
            // si no es uno de estos parametros no cuenta
            std::cout << "[ERROR!] Parametro no valido: " << nombre_parametro << std::endl;
            return; 
        }
    }

    // --------------- VALIDACIONES
    //disk size > 0
    if (disk_size <= 0) {
        std::cout << "[ERROR!] El disk_size debe ser mayor que 0" << std::endl;
        return;
    }
    // debe haber un path
    if (disk_path.empty()) {
        std::cout << "[ERROR!] MKDISK requiere el parametro path" << std::endl;
        return;
    }

    // si no viene fit se va un default
    if (fit.empty()) {
        fit = "ff"; 
    }
    // si no viene unit se va default
    if (disk_unit.empty()) {
        disk_unit = "m"; 
    }

    // pasar de string a char el fit
    if (fit == "ff") {
        fit_char = 'f';
    } else if (fit == "bf") {
        fit_char = 'b';
    } else if (fit == "wf") {
        fit_char = 'w';
    } else {
        std::cout << "[ERROR!] Tipo de Fit NO VALIDO - SOLO SE ACEPTA: ff, bf, wf" << std::endl;
        return;
    }

    // AJUSTAR TAMAÑO A BYTES
    int disk_size_bytes = 0;
    if (disk_unit == "m") {
        disk_size_bytes = disk_size * 1024 * 1024;
    } else if (disk_unit == "k") {
        disk_size_bytes = disk_size * 1024;
    } else {
        std::cout << "[ERROR!] Tipo de Unit NO VALIDO - SOLO SE ACEPTA: k, m" << std::endl;
        return;
    }

    //----------- CREACION DEL DISCO
    // 1. VERIFICAR QUE NO EXISTA
    if (std::filesystem::exists(disk_path)) {
        std::cout << "[ERROR!] el disk_file ya exioste en ese path"<< std::endl;
        return;
    }

    // 2. crear los directoris si no existe
    try {
        std::filesystem::path disk_path_fs(disk_path);
        std::filesystem::path parent = disk_path_fs.parent_path();
        if (!parent.empty()) {
            // si el padre no existe crearlo
            std::filesystem::create_directories(parent);
        }
    } catch (const std::exception &e) {
        std::cout << "[ERROR!] ERROR AL CREAR LOS DIRECTORIOS " << e.what() << std::endl;
        return;
    }

    // 3. CREAR EL disk_file
    disk_file = fopen(disk_path.c_str(), "wb+");
    if (disk_file == nullptr) {
        std::cout << "[ERROR!] No se pudo crear el disco" << std::endl;
        return;
    }

    // 4. dimensionarlo, se pone el punteero al ultimo byte y se escribe un byte vacio
    if (disk_size_bytes > 0) {
        if (fseek(disk_file, disk_size_bytes - 1, SEEK_SET) != 0) {
            std::cout << "[ERROR!] Error al dimensionar el disk_file" << std::endl;
            fclose(disk_file);
            return;
        }
        fwrite(&vacio, sizeof(vacio), 1, disk_file);
    }

    //------------------- llenar el MBRO
    mbr.mbr_tamano = disk_size_bytes;
    mbr.mbr_dsk_signature = std::rand() % 7777;
    mbr.mbr_fecha_creacion = std::time(nullptr);
    mbr.dsk_fit = fit_char;

    for (int i = 0; i < 4; i++) {
        std::strcpy(mbr.mbr_partition[i].part_name, "");
        mbr.mbr_partition[i].part_status = '0';
        mbr.mbr_partition[i].part_s = 0;
        mbr.mbr_partition[i].part_fit = fit_char;
        mbr.mbr_partition[i].part_start = -1;
    }

    // ---------- ESRRIBIR EL MBR AL DISK FILE
    fseek(disk_file, 0, SEEK_SET);
    fwrite(&mbr, sizeof(MBR), 1, disk_file);
    fclose(disk_file);

    std::cout << "[SUCCESS] disk_file creado correctamente: " << disk_path << std::endl;
}
