#include <algorithm>
#include <iostream>
#include <string>

#include "analizador.h"

int main() {
    // LOOP PRINCIPAL DEL ANALIZADOD
    //keep going es el loop
    bool keep_going = true;

    //MAIN= ES EL COMANDO
    std::string command;

    std::cout << "******************************************" << std::endl;
    std::cout << " LOOP PRINCIPAL DEL ANALIZADOR " << std::endl;
    std::cout << " >>> exit para salir" << std::endl;
    std::cout << "******************************************" << std::endl;

    //LOOP, MIENTRAS NO SE INDIQUE EXIT SERA TRUE
    while (keep_going) {

        // SE ESCRIBE EL COMANDO
        std::cout << std::endl << "--> ";
        if (!std::getline(std::cin, command)) {
            break;
        }

        //exit
        std::string command_normalizado = command;
        std::transform(command_normalizado.begin(), command_normalizado.end(), command_normalizado.begin(),
                    [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (command_normalizado == "exit") {
            keep_going = false;
            continue;
        }

        run_command(command);
    }

    std::cout << "********************FIN************************" << std::endl;
    return 0;
}
