#include "analizador.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>

#include "mkdisk.h"
#include "rmdisk.h"


void lexical_analysis(std::string &string_input, std::vector<std::string> &parametros) {
    //forzar que se cierre
    string_input += " ";

    int clasificacion = 0;

    std::string string_temporal;


    //clasificacionS
    // 0: INICIO DE ACA SE VA A LOS OTROS
    // 1: SI ES UNA LETRA (comando) 
    // 2: SI ES UN PARAMETRO
    // 4: SI ES UN PARAMETRO ENTRE COMILLAS
    // 3: COMENTARIO

    for (int i = 0; i < static_cast<int>(string_input.length()); i++) {
        switch (clasificacion) {
            case 0:
                if (std::isalpha(static_cast<unsigned char>(string_input[i]))) {
                    // Si es una letra puede ser un comando 
                    clasificacion = 1;
                    string_temporal += string_input[i];
                } else if (string_input[i] == '>') {
                    // parametro
                    clasificacion = 2;
                } else if (string_input[i] == '#') {
                    // comentario
                    clasificacion = 3;
                }
                break;
            case 1:
            //COMANDO : analliza hasta que encuentra espacio
                if (string_input[i] == ' ') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 2:
            //PARAMETRO : analiza hasta espacio o comillas
                if (string_input[i] == '"') {
                    // CADENA COMO PARAMETRO
                    clasificacion = 4;
                } else if (string_input[i] == ' ') {
                    // VALORES NORMALES COMO PARAMETRO
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 4:
            //PARAMETRO ENTRE COMILLAS : analiza hasta encontrar comilla de cierre
                if (string_input[i] == '"') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 3:
            //COMENTARIO : ignorar todo hasta el final de la linea
                break;
        }
    }
}


//------------------------------------ MAIN LOOP ----------------------------------------------

void run_command(std::string &string_input) {
    std::vector<std::string> parametros;
    //PARAMETROS ES EL VECTOR EN DONDE SE VAN A GUARDAR
    lexical_analysis(string_input, parametros);

    if (parametros.empty()) {
        std::cout << ">>[!] VACIO/COMMENTARIOO" << std::endl;
        return;
    }

    std::string tipo_command = parametros[0];
    std::transform(tipo_command.begin(), tipo_command.end(), tipo_command.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });


    //--------------------- LLAMADO A FUNCIONES SEGUN COMANDO
    if (tipo_command == "mkdisk") {
        mkdisk(parametros);
    } else if (tipo_command == "rmdisk") {
        rmdisk(parametros);
    } else {
        std::cout << ">>[ERROR] El comando escrito no se reconoce" << std::endl;
    }
}
