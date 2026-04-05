#include "analizador.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>

#include "structs.h"
#include "mkdisk.h"
#include "rmdisk.h"
#include "fdisk.h"
#include "mount.h"
#include "mkfs.h"
#include "rep.h"
#include "journaling.h"

namespace {
std::vector<disco> discos_montados;
}

void lexical_analysis(std::string &string_input, std::vector<std::string> &parametros) {
    string_input += " ";

    int clasificacion = 0;
    std::string string_temporal;

    // 0: inicio
    // 1: comando
    // 2: parametro
    // 3: comentario
    // 4: parametro entre comillas
    for (int i = 0; i < static_cast<int>(string_input.length()); i++) {
        switch (clasificacion) {
            case 0:
                if (std::isalpha(static_cast<unsigned char>(string_input[i]))) {
                    clasificacion = 1;
                    string_temporal += string_input[i];
                } else if (string_input[i] == '>') {
                    clasificacion = 2;
                } else if (string_input[i] == '#') {
                    clasificacion = 3;
                }
                break;
            case 1:
                if (string_input[i] == ' ') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 2:
                if (string_input[i] == '"') {
                    clasificacion = 4;
                } else if (string_input[i] == ' ') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 4:
                if (string_input[i] == '"') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 3:
                break;
        }
    }
}

void run_command(std::string &string_input) {
    std::vector<std::string> parametros;
    lexical_analysis(string_input, parametros);

    if (parametros.empty()) {
        std::cout << ">>[!] VACIO/COMMENTARIOO" << std::endl;
        return;
    }

    std::string tipo_command = parametros[0];
    std::transform(tipo_command.begin(), tipo_command.end(), tipo_command.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (tipo_command == "mkdisk") {
        mkdisk(parametros);
    } else if (tipo_command == "rmdisk") {
        rmdisk(parametros);
    } else if (tipo_command == "fdisk") {
        fdisk(parametros);
    } else if (tipo_command == "mount") {
        mount(parametros, discos_montados);
    } else if (tipo_command == "mkfs") {
        mkfs(parametros, discos_montados);
    } else if (tipo_command == "rep") {
        rep(parametros, discos_montados);
    } else if (tipo_command == "journaling") {
        journaling(parametros, discos_montados);
    } else {
        std::cout << ">>[ERROR] El comando escrito no se reconoce" << std::endl;
    }
}
