#ifndef FDISK
#define FDISK


#include "structs.h"


#include <iostream>
#include <vector>
#include <algorithm>

// recibe parametros que van algo tipo:
//parametros[0] = "mkdisk"
//parametros[1] = "size=10"
//parametros[2] = "path=/home/user/disco.dk"
void fdisk(std::vector<std::string> &parametros);


// CREA UNA NUEVA PARTICION- SIZE, TIPO (primaria, logica, ex), PATH del disco, nombre de la particion, fit (best, worst, first)
void particionar(int &size, char &tipo, std::string &ruta, std::string &nombre, char &fit);

#endif