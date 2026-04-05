#include "mkfs.h"

#include <cstdio>
#include <cstring>

namespace {
void copy_text(char *dest, size_t dest_size, const std::string &source) {
    std::memset(dest, 0, dest_size);
    std::strncpy(dest, source.c_str(), dest_size - 1);
}
}

void mkfs(std::vector<std::string> &parametros, std::vector<disco> &discos) {
    // la data
    std::string tipo;      // opcional, siempre full
    std::string id;        // ID DE LA PARTICION
    std::string fs = "2fs"; // ext2 por default

    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() != 2) {
            std::cout << "[ERROR] PARAMETRO NO VALIDO " << command_data
                      << " (debe ser clave=valor, ej: id=0070disco1 type=full fs=3fs)" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        std::transform(valor_parametro.begin(), valor_parametro.end(), valor_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (nombre_parametro == "type") {
            tipo = valor_parametro;
        } else if (nombre_parametro == "id") {
            id = output_data[1];
        } else if (nombre_parametro == "fs") {
            fs = valor_parametro;
        } else {
            std::cout << "[ERROR] PARAMETRO NO VALIDO, SOLO SE ACEPTA ID, TYPE Y FS" << std::endl;
            return;
        }
    }

    if (id.empty()) {
        std::cout << "[ERROR] MKFS REQUIERE EL PARAMETRO ID" << std::endl;
        return;
    }

    if (!tipo.empty() && tipo != "full") {
        std::cout << "[ERROR] SOLO SE PUEDE FORMATEO: FULL" << std::endl;
        return;
    }

    if (fs != "2fs" && fs != "3fs") {
        std::cout << "[ERROR] FS SOLO ACEPTA 2FS O 3FS" << std::endl;
        return;
    }

    const bool use_ext3 = (fs == "3fs");

    // Sacar el nombre del disco usando el id, se quitan los digitos iniciales
    size_t position_digits = 0;
    while (position_digits < id.size() && std::isdigit(static_cast<unsigned char>(id[position_digits]))) {
        ++position_digits;
    }

    std::string diskName = id.substr(position_digits);

    // BUSCAR EL DISCO EN EL VECTOR DISCO
    int posDisco = -1;
    for (int i = 0; i < static_cast<int>(discos.size()); ++i) {
        if (discos[i].nombre == diskName) {
            posDisco = i;
            break;
        }
    }

    if (posDisco == -1) {
        std::cout << "[ERROR] EL DISCO NO ESTA MONTADO, DEBE SER MONTADO PARA USAR" << std::endl;
        return;
    }

    disco &mounted_disk = discos[posDisco];
    int posParticion = -1;

    for (int i = 0; i < static_cast<int>(mounted_disk.particiones.size()); ++i) {
        if (mounted_disk.particiones[i].id == id) {
            posParticion = i;
            break;
        }
    }

    if (posParticion == -1) {
        std::cout << "[ERROR] LA PARTICION NO EXISTE, REVISAR ID" << std::endl;
        return;
    }

    montada &mkfs_formateo = mounted_disk.particiones[posParticion];
    FILE *disk_file = fopen(mounted_disk.ruta.c_str(), "r+b");

    if (!disk_file) {
        std::cout << "[ERROR] NO SE PUDO ABRIR EL ARCHIVO DEL DISCO" << std::endl;
        return;
    }

    int particion_inicio = 0;
    int p_size = 0;

    if (mkfs_formateo.posMBR != -1) {
        MBR mbr;
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        particion_inicio = mbr.mbr_partition[mkfs_formateo.posMBR].part_start;
        p_size = mbr.mbr_partition[mkfs_formateo.posMBR].part_s;
    } else {
        EBR ebr;
        fseek(disk_file, mkfs_formateo.posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        particion_inicio = ebr.part_start;
        p_size = ebr.part_s;
    }

    sbloque nuevo;

    int denominator = 196 + static_cast<int>(sizeof(inodo));
    if (use_ext3) {
        denominator += static_cast<int>(sizeof(registro_journal));
    }

    int n = static_cast<int>(std::floor((p_size - static_cast<int>(sizeof(sbloque))) / denominator));
    if (n < 2) {
        std::cout << "[ERROR] LA PARTICION ES MUY PEQUENA PARA FORMATEAR" << std::endl;
        fclose(disk_file);
        return;
    }

    nuevo.s_filesystem_type   = use_ext3 ? 3 : 2;
    nuevo.s_inodes_count      = n;
    nuevo.s_blocks_count      = n * 3;
    nuevo.s_free_blocks_count = (n * 3) - 2;
    nuevo.s_free_inodes_count = n - 2;

    nuevo.s_mtime             = time(nullptr);
    nuevo.s_umtime            = time(nullptr);
    nuevo.s_mnt_count         = 1;
    nuevo.s_magic             = 0xEF53;
    nuevo.s_inode_s           = sizeof(inodo);
    nuevo.s_block_s           = sizeof(barchivos);

    if (use_ext3) {
        nuevo.s_journal_start = particion_inicio + static_cast<int>(sizeof(sbloque));
        nuevo.s_journal_count = 1;
        nuevo.s_journal_max = n;
        nuevo.s_bm_inode_start = nuevo.s_journal_start + (n * static_cast<int>(sizeof(registro_journal)));
    } else {
        nuevo.s_journal_start = -1;
        nuevo.s_journal_count = 0;
        nuevo.s_journal_max = 0;
        nuevo.s_bm_inode_start = particion_inicio + static_cast<int>(sizeof(sbloque));
    }

    nuevo.s_bm_block_start = nuevo.s_bm_inode_start + (n * static_cast<int>(sizeof(char)));
    nuevo.s_inode_start = nuevo.s_bm_block_start + ((n * static_cast<int>(sizeof(char))) * 3);
    nuevo.s_block_start = nuevo.s_inode_start + (n * static_cast<int>(sizeof(inodo)));

    nuevo.s_first_ino = nuevo.s_inode_start + (2 * static_cast<int>(sizeof(inodo)));
    nuevo.s_first_blo = nuevo.s_block_start + (2 * static_cast<int>(sizeof(barchivos)));

    fseek(disk_file, particion_inicio, SEEK_SET);
    fwrite(&nuevo, sizeof(sbloque), 1, disk_file);

    if (use_ext3) {
        registro_journal vacio_journal{};
        fseek(disk_file, nuevo.s_journal_start, SEEK_SET);
        for (int i = 0; i < nuevo.s_journal_max; ++i) {
            fwrite(&vacio_journal, sizeof(registro_journal), 1, disk_file);
        }

        registro_journal primer_registro{};
        copy_text(primer_registro.j_command, sizeof(primer_registro.j_command), "mkfs");
        copy_text(primer_registro.j_path, sizeof(primer_registro.j_path), mounted_disk.ruta);
        copy_text(primer_registro.j_content, sizeof(primer_registro.j_content), "filesystem=3fs");
        primer_registro.j_date = time(nullptr);
        primer_registro.j_owner = 1;

        fseek(disk_file, nuevo.s_journal_start, SEEK_SET);
        fwrite(&primer_registro, sizeof(registro_journal), 1, disk_file);
    }

    char zero_char  = '0';
    char vacio = '\0';

    fseek(disk_file, nuevo.s_bm_inode_start, SEEK_SET);
    fwrite(&zero_char, sizeof(char), nuevo.s_inodes_count, disk_file);

    fseek(disk_file, nuevo.s_bm_block_start, SEEK_SET);
    fwrite(&zero_char, sizeof(char), nuevo.s_blocks_count, disk_file);

    fseek(disk_file, nuevo.s_block_start, SEEK_SET);
    fwrite(&vacio, sizeof(char), nuevo.s_blocks_count * static_cast<int>(sizeof(barchivos)), disk_file);

    fseek(disk_file, nuevo.s_inode_start, SEEK_SET);
    fwrite(&vacio, sizeof(char), nuevo.s_inodes_count * static_cast<int>(sizeof(inodo)), disk_file);

    // inodo 0 y bloque 0 para la raiz
    char uno = '1';
    fseek(disk_file, nuevo.s_bm_inode_start, SEEK_SET);
    fwrite(&uno, sizeof(char), 1, disk_file);

    char c = 'c';
    fseek(disk_file, nuevo.s_bm_block_start, SEEK_SET);
    fwrite(&c, sizeof(char), 1, disk_file);

    inodo nuevo_inodo;
    nuevo_inodo.i_uid   = 1;
    nuevo_inodo.i_gid   = 1;
    nuevo_inodo.i_s     = 0;
    nuevo_inodo.i_atime = time(nullptr);
    nuevo_inodo.i_ctime = time(nullptr);
    nuevo_inodo.i_mtime = time(nullptr);
    for (int i = 0; i < 15; ++i) {
        nuevo_inodo.i_block[i] = -1;
    }
    nuevo_inodo.i_block[0] = 0;
    nuevo_inodo.i_type     = '0';
    nuevo_inodo.i_perm     = 777;

    fseek(disk_file, nuevo.s_inode_start, SEEK_SET);
    fwrite(&nuevo_inodo, sizeof(inodo), 1, disk_file);

    bcarpetas nueva_ncarpeta;
    for (int i = 0; i < 4; ++i) {
        std::strcpy(nueva_ncarpeta.b_content[i].b_name, "-");
        nueva_ncarpeta.b_content[i].b_inodo = -1;
    }

    std::strcpy(nueva_ncarpeta.b_content[0].b_name, ".");
    nueva_ncarpeta.b_content[0].b_inodo = 0;

    std::strcpy(nueva_ncarpeta.b_content[1].b_name, "..");
    nueva_ncarpeta.b_content[1].b_inodo = 0;

    std::strcpy(nueva_ncarpeta.b_content[2].b_name, "users.txt");
    nueva_ncarpeta.b_content[2].b_inodo = 1;

    fseek(disk_file, nuevo.s_block_start, SEEK_SET);
    fwrite(&nueva_ncarpeta, sizeof(bcarpetas), 1, disk_file);

    // inodo y bloque para users.txt
    fseek(disk_file, nuevo.s_bm_inode_start + 1, SEEK_SET);
    fwrite(&uno, sizeof(char), 1, disk_file);

    char a = 'a';
    fseek(disk_file, nuevo.s_bm_block_start + 1, SEEK_SET);
    fwrite(&a, sizeof(char), 1, disk_file);

    std::string contenido = "1,G,root\n1,U,root,root,123\n";

    nuevo_inodo.i_uid   = 1;
    nuevo_inodo.i_gid   = 1;
    nuevo_inodo.i_s     = static_cast<int>(contenido.size());
    nuevo_inodo.i_atime = time(nullptr);
    nuevo_inodo.i_ctime = time(nullptr);
    nuevo_inodo.i_mtime = time(nullptr);
    for (int i = 0; i < 15; ++i) {
        nuevo_inodo.i_block[i] = -1;
    }
    nuevo_inodo.i_block[0] = 1;
    nuevo_inodo.i_type     = '1';
    nuevo_inodo.i_perm     = 777;

    fseek(disk_file, nuevo.s_inode_start + sizeof(inodo), SEEK_SET);
    fwrite(&nuevo_inodo, sizeof(inodo), 1, disk_file);

    barchivos nuevo_barchivo;
    std::memset(nuevo_barchivo.b_content, 0, sizeof(nuevo_barchivo.b_content));
    std::strncpy(nuevo_barchivo.b_content, contenido.c_str(), sizeof(nuevo_barchivo.b_content) - 1);

    fseek(disk_file, nuevo.s_block_start + sizeof(barchivos), SEEK_SET);
    fwrite(&nuevo_barchivo, sizeof(barchivos), 1, disk_file);

    std::cout << "[SUCCESS] La particion fue formateada correctamente en "
              << (use_ext3 ? "EXT3" : "EXT2") << "." << std::endl;
    fclose(disk_file);
}
