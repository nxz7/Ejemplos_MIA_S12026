#include "analizador.h"

#include <cctype>
#include <iostream>
#include <vector>


void lexical_analysis(std::string &string_input, std::vector<std::string> &parametros) {

    // forzar el cierre
    string_input += " ";

    int clasificacion = 0;
    std::string string_temporal;


    //clasificacion
    //0: INICIO DE ACA SE VA A LOS OTROS
    //1: SI ES UNA LETRA(comando)
    //2: si es un parametro
    //4: si es esta entre comillas
    //3: comentarios

    //mkdisk >size=10 >path="/tmp/Disco1.dsk"

    for (int i = 0; i < static_cast<int>(string_input.length()); i++) {
        switch (clasificacion) {
            case 0:
            // si es una letra
                if (std::isalpha(static_cast<unsigned char>(string_input[i]))) {
                    clasificacion =1;
                    string_temporal += string_input[i];
                }else if(string_input[i] == '>'){
                    //parametro
                    clasificacion =2;
                } else if (string_input[i] == '#') {
                    // comentario
                    clasificacion = 3;
                }
                break;
             
            case 1:
            // comando: aca se analiza hasta que se encuentra un espacio
                if (string_input[i] == ' ') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 2:
            //PARAMETRO, > y analiza hasta que encuenta un espacio
                if (string_input[i] == '"') {
                    // CADENA COMO PARAMETRO
                    clasificacion = 4;
                } else if (string_input[i] == ' ') {
                    // valores sin comillas
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 4:
            // parametros entre comillas : analiza hasta encontrar el cierre
                 if (string_input[i] == '"') {
                    parametros.push_back(string_temporal);
                    string_temporal.clear();
                    clasificacion = 0;
                } else {
                    string_temporal += string_input[i];
                }
                break;
            case 3:
            // COMENTARIO---- IGNORAR
                 break;
}
}
}

void run_command(std::string &string_input) {
// Es el vector en donde se van gurdando
std::vector<std::string> parametros;

lexical_analysis(string_input, parametros);

    if (parametros.empty()) {
        std::cout << ">>[ERROR-OJO!] NO DE RECONOCIERON LOS PARAMETROS" << std::endl;
        return;
    }

    // listas los parametros reconocidos
    std::cout << ">>[SUCCESS] RECONOCIDO:" << std::endl;
    for (size_t i = 0; i < parametros.size(); ++i) {
        //mostrar lo reconocido
        std::cout << "   [" << i << "] --- " << parametros[i] << std::endl;
    }

}