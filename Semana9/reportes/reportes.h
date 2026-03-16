#ifndef REPORTES_CLASE5
#define REPORTES_CLASE5

#include "../structs.h"
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

void reporte_mbr(std::vector<disco> &discos, int posDisco, std::string &ruta);
void reporte_disk(std::vector<disco> &discos, int posDisco, std::string &ruta);
void reporte_inode(std::vector<disco> &discos, int posDisco, int posParticion, std::string &ruta);
void reporte_bm_inode(std::vector<disco> &discos, int posDisco, int posParticion, std::string &ruta);

#endif
