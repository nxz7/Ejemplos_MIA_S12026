#ifndef RMDISK
#define RMDISK

//Locales
#include "structs.h"
#include <string>
#include <vector>

// recibe parametros que van algo tipo:
//parametros[0] = "mkdisk"
//parametros[1] = "size=10"
//parametros[2] = "path=/home/user/disco.dk"
void rmdisk(std::vector<std::string> &parametros);

#endif
