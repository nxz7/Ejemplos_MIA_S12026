#include "rep.h"
#include "reportes/reportes.h"

void rep(std::vector<std::string> &parametros, std::vector<disco> &discos) {

    //DATOS DEL COMANDO
    std::string path;     
    std::string reporte_name;   
    std::string id;       

    // LEER LOS PARAMETROS
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &temp = parametros[i];

        std::vector<std::string> output_data( std::sregex_token_iterator(temp.begin(), temp.end(), igual, -1), std::sregex_token_iterator() );

        if (output_data.size() != 2) {
            std::cout << "[ERROR] PARAMETRO NO VALIDO " << temp   << " (formato esperado: clave=valor)" << std::endl;
            return;
        }

        std::string nombre_parametro   = output_data[0];
        std::string valor_parametro = output_data[1];

        // nomrlaizar a minuscula
        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });

                       //asignar el valor al parametro correspondiente
        if (nombre_parametro == "path") {
            path = valor_parametro;
        } else if (nombre_parametro == "name") {
            reporte_name = valor_parametro;
        } else if (nombre_parametro == "id") {
            id = valor_parametro;
        } else {
            std::cout << "[ERROR] Parametro no valido:solo se permite path, name y id" << std::endl;
            return;
        }
    }

    // ----------------validacion basica
    if (path.empty() || reporte_name.empty() || id.empty()) {
        std::cout << "[ERROR] LOS PAREMETROS OBLIGATORIOS SON PATH, NAME Y ID" << std::endl;
        return;
    }

    // SACAR EL NOMBRE SEGUN EL ID QUE MANDA
    //sacar los digitos hasta que tenga la letra
    size_t position_digit = 0;
    while (position_digit < id.size() && std::isdigit(static_cast<unsigned char>(id[position_digit]))) {
        ++position_digit;
    }
    std::string diskName = id.substr(position_digit);  

    // buscar el disco entre la tabla de discos montados
    int position_disk = -1;
    for (size_t i = 0; i < discos.size(); ++i) {
        if (discos[i].nombre == diskName) {
            position_disk = static_cast<int>(i);
            break;
        }
    }
    //REVISAR SI SI SE ENCONTRO
    if (position_disk == -1) {
        std::cout << "[ERROR] EL DISCO  '" << diskName << "' NO ESTA MONTADO" << std::endl;
        return;
    }

    // BUSCAR LA PARTICION SI ES NECESARIO
    disco &d = discos[position_disk];
    int partition_position = -1;

    for (size_t i = 0; i < d.particiones.size(); ++i) {
        if (d.particiones[i].id == id) {
            partition_position = static_cast<int>(i);
            break;
        }
    }

    if (partition_position == -1) {
        std::cout << "[ERROR] NO EXISTE la particion ID: "<< id  << std::endl;
        return;
    }

    // PATH DE SALIDA, VER LO DE LOS DIRECTORIOS
    // SE MANDA EL FULL PATH PERO LO QUE SE VA A CREAR ES LA CARPETA PADRE SI NO EXISTE
    std::filesystem::path outPath(path);
    try {
        if (outPath.has_parent_path()) {
            std::filesystem::create_directories(outPath.parent_path());
        }
    } catch (const std::exception &e) {
        std::cout << "[ERROR]NO SE PUDO CREAR EL DIRECTORIO " << e.what() << std::endl;
        return;
    }

    // AGREGAR NORMALIZACION AL NOMBRE PARA QUE SE RECONOZCA
    std::transform(reporte_name.begin(), reporte_name.end(), reporte_name.begin(),[](unsigned char c){ return std::tolower(c); });

    if (reporte_name == "mbr") {
        reporte_mbr(discos, position_disk, path);
    } else if (reporte_name == "disk") {
        reporte_disk(discos, position_disk, path);
    } else if (reporte_name == "inode") {
        reporte_inode(discos, position_disk, partition_position, path);
    }else {
        std::cout << "[ERROR]EL NOEMBRE DEL REPORTE NO ES VALIDO " << std::endl;
    }
}
