#ifndef RMDISK
#define RMDISK

//Locales
#include "structs.h"
#include <string>
#include <vector>

// recibe parametros que van algo tipo:
//parametros[0] = "rmdisk"
//parametros[1] = "path=/home/user/disco.dk"
void rmdisk(std::vector<std::string> &parametros);

#endif
