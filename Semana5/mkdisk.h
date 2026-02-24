#ifndef MKDISK
#define MKDISK


#include "structs.h"
#include <string>
#include <vector>

// recibe parametros que van algo tipo:
//parametros[0] = "mkdisk"
//parametros[1] = "size=10"
//parametros[2] = "path=/home/user/disco.dk"
void mkdisk(std::vector<std::string> &parametros);

#endif
