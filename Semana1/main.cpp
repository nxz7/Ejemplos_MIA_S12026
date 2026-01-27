#include <algorithm>
#include <iostream>
#include <string>
#include "analizador.h"

int main() {
// Loop principal

bool keep_going = true;

    std::string command;

    std::cout << "******************************************" << std::endl;
    std::cout << " LOOP PRINCIPAL DEL ANALIZADOR " << std::endl;
    std::cout << " >>> exit para salir" << std::endl;
    std::cout << "******************************************" << std::endl;

    while (keep_going) {

        // SE ESCRIBE EL COMANDO
        std::cout << std::endl << "--> ";
        // con get line lee toda la linea hasta el enter
        if (!std::getline(std::cin, command)) {
            break;
        }

        //exit
        std::string command_normalizado = command;
        std::transform(command_normalizado.begin(), command_normalizado.end(), command_normalizado.begin(),
                    [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        //Exit 
        //EXIT
        //MKDISK, mkdisk, MkDisk

        if (command_normalizado == "exit") {
            keep_going = false;
            continue;
        }

        run_command(command);

 }

     std::cout << "********************FIN************************" << std::endl;
    return 0;

 }