#include "mkfs.h"

void mkfs(std::vector<std::string> &parametros, std::vector<disco> &discos) {
    // la data
    std::string tipo;   // es opcional pero siempre es full
    std::string id;     // ID DE LA PARTICION, de aca se saca el nombre del disco

    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        //separa la data, para obtener los parametros y valores
        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() != 2) {
            std::cout << "[ERROR] PARAMETRO NO VALIDO " << command_data << " (debe ser clave=valor, ej: id=0070disco1 type=full)" << std::endl;
            return;
        }

        // saca la data
        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        //todo a lower
        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (nombre_parametro == "type") {
            tipo = valor_parametro;
        } else if (nombre_parametro == "id") {
            id = valor_parametro;
        } else {
            std::cout << "[ERROR] PARAMETRO NO VALIDO, SOLO SE ACEPTA ID Y TYPE" << std::endl;
            return;
        }
    }

    // VALIDACIONES
    // ID ES OBLIGATORIO
    if (id.empty()) {
        std::cout << "[ERROR] MKFS REQUIERE EL PARAMETRO ID" << std::endl;
        return;
    }

    // normalizar el type por si hay errore y ver que solo acepten null o full
    std::transform(tipo.begin(), tipo.end(), tipo.begin(),[](unsigned char c){ return std::tolower(c); });

    if (!tipo.empty() && tipo != "full") {
        std::cout << "[ERROR] SOLO SE PUEDE FORMATEO: FULL" << std::endl;
        return;
    }

    // Sacr el nombre del disco usando el id, se quitan los digitos iniciales
    size_t position_digits = 0;
    while (position_digits < id.size() && std::isdigit(static_cast<unsigned char>(id[position_digits]))) {
        ++position_digits;
    }

    //se quita desde el iniciio de las letras hasta el final
    std::string diskName = id.substr(position_digits);  

    // BUSCAR EL DISCO EN EL VECTOR DISCO
    int posDisco = -1;
    for (int i = 0; i < static_cast<int>(discos.size()); ++i) {
        if (discos[i].nombre == diskName) {
            //sacara la posicion del disco montado
            posDisco = i;
            break;
        }
    }

    if (posDisco == -1) {
        std::cout << "[ERROR]EL DISCO NO ESTA MONTADO, DEBE SER MONTADO PARA USAR" << std::endl;
        return;
    }

    // BUSCAR LA PARTICION DENTRO DEL DISCO MONTADO, ACA SE USA EL ID
    disco &mounted_disk = discos[posDisco];
    int posParticion = -1;

    for (int i = 0; i < static_cast<int>(mounted_disk.particiones.size()); ++i) {
        if (mounted_disk.particiones[i].id == id) {
            posParticion = i;
            break;
        }
    }
    if (posParticion == -1) {
        std::cout << "[ERROR]LA PARTICION NO ESISTE, REVISAR ID" << std::endl;
        return;
    }

    // --------------------------------- ABRIR EL ARCHIVO
    montada &mkfs_formateo = mounted_disk.particiones[posParticion];
    FILE *disk_file = fopen(mounted_disk.ruta.c_str(), "r+b");

    if (!disk_file) {
        std::cout << "[ERROR]NO SE PUDO ABRIR EL ARCHIVO DEL DISCO" << std::endl;
        return;
    }

    // DETERMINIAR EL INICIO Y EL TAMAÑO DE LA PARTICION
    int particion_inicio = 0;
    int p_size = 0;

    if (mkfs_formateo.posMBR != -1) {
        // en caso sea primiaria/extendida
        MBR mbr;
        fseek(disk_file, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, disk_file);
        particion_inicio = mbr.mbr_partition[mkfs_formateo.posMBR].part_start;
        p_size    = mbr.mbr_partition[mkfs_formateo.posMBR].part_s;
    } else {
        // particion logica
        EBR ebr;
        fseek(disk_file, mkfs_formateo.posEBR, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk_file);
        particion_inicio = ebr.part_start;
        p_size    = ebr.part_s;
    }

    //---------------------------------------SUPERBLOQUE
    //SUPERBLOQUE Y CALCULAR LOS DATOS
    sbloque nuevo;
    // NUMERO MAXIMO DE INODOS
    //n numero de inodos
    int n = static_cast<int>(std::floor((p_size - sizeof(sbloque)) / (196 + sizeof(inodo))));

    nuevo.s_filesystem_type   = 2;
    nuevo.s_inodes_count      = n;
    // cada inodo tiene 3 bloques
    nuevo.s_blocks_count      = n * 3;
    // quito dos bloques porque se va a usar para la raiz y users.txt
    //bloque usara: raiz y users.txt
    nuevo.s_free_blocks_count = (n * 3) - 2;   
    //dos inodos: carpeta raiz y users.txt
    nuevo.s_free_inodes_count = n - 2;     

    nuevo.s_mtime             = time(nullptr);
    nuevo.s_umtime            = time(nullptr);
    nuevo.s_mnt_count         = 1;
    nuevo.s_magic             = 0xEF53;
    nuevo.s_inode_s           = sizeof(inodo);
    nuevo.s_block_s           = sizeof(barchivos);

    // doonde inician

    nuevo.s_bm_inode_start = particion_inicio + sizeof(sbloque);
    nuevo.s_bm_block_start = nuevo.s_bm_inode_start + (n * sizeof(char));

    nuevo.s_inode_start = nuevo.s_bm_block_start + ((n * sizeof(char)) * 3);
    nuevo.s_block_start = nuevo.s_inode_start + (n * sizeof(inodo));


    //primer inodo y bloque libre
    nuevo.s_first_ino = nuevo.s_inode_start + (2 * sizeof(inodo));
    nuevo.s_first_blo = nuevo.s_block_start + (2 * sizeof(barchivos));

    // ESCRIBIR LA DATA - 1,superbloqeu
    fseek(disk_file, particion_inicio, SEEK_SET);
    fwrite(&nuevo, sizeof(sbloque), 1, disk_file);

    // -------------------BITMAPS, INODOS Y BLOQUES
    char zero_char  = '0';
    char vacio = '\0';

    // BITMAP DE INODOS, TODO EN cero 2.bitmap de inodo
    fseek(disk_file, nuevo.s_bm_inode_start, SEEK_SET);
    fwrite(&zero_char, sizeof(char), nuevo.s_inodes_count, disk_file);

    // BITMAP DE BLOQUES, TODO EN cero 3.bitmap de bloques
    fseek(disk_file, nuevo.s_bm_block_start, SEEK_SET);
    fwrite(&zero_char, sizeof(char), nuevo.s_blocks_count, disk_file);

    // LIMPIAR BLOQUES - dejarlos vacios
    fseek(disk_file, nuevo.s_block_start, SEEK_SET);
    fwrite(&vacio, sizeof(char), nuevo.s_blocks_count * sizeof(barchivos), disk_file);

    // LIMPIAR INODOS - dejarlos vacios
    fseek(disk_file, nuevo.s_inode_start, SEEK_SET);
    fwrite(&vacio, sizeof(char), nuevo.s_inodes_count * sizeof(inodo), disk_file);

    //-----------------------------------------
    /*
   >inodo (metadata de archivo o carpeta-no el archivo)
    contiene la info de un archivo o carpeta, no se guarda el nombre solo su metadata
    como posicion, permiso, dueño, grupo, etx
    i_block[15] = puntero a los bloques de datos(barchivos o bcarpetas)
    >BLOQUE
    >>barchivos (guarda el contenido de un archivo)
    >>bcarpetas guarda el contenido, inclue el contenido.
    >>content dentro de carpteas y guarda el nombre y el inodo al que apunta

    */


    //-----------------------------------------
    // USAR EL PRIMER INODO Y BLOQUE PARA LA RAIZ -- 1=USADO
    // inodo 0
    char uno = '1';
    fseek(disk_file, nuevo.s_bm_inode_start, SEEK_SET);
    fwrite(&uno, sizeof(char), 1, disk_file);

    // bloque 0 
    // tipo C, tipo carpeta
    char c = 'c';
    fseek(disk_file, nuevo.s_bm_block_start, SEEK_SET);
    fwrite(&c, sizeof(char), 1, disk_file);

    // INODO RAIZ
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
    nuevo_inodo.i_block[0] = 0;    // usa el bloque 0
    nuevo_inodo.i_type     = '0';  // directorio 0
    nuevo_inodo.i_perm     = 777;

    fseek(disk_file, nuevo.s_inode_start, SEEK_SET);
    fwrite(&nuevo_inodo, sizeof(inodo), 1, disk_file);

    // -- CREAR EL BLOQUE DE LA RAIZ
    bcarpetas nueva_ncarpeta;

    // inicializar entradas como vacías
    for (int i = 0; i < 4; ++i) {
        std::strcpy(nueva_ncarpeta.b_content[i].b_name, "-");
        nueva_ncarpeta.b_content[i].b_inodo = -1;
    }

    // "." ES PARA QUE APUNTE A EL MISMO
    std::strcpy(nueva_ncarpeta.b_content[0].b_name, ".");
    nueva_ncarpeta.b_content[0].b_inodo = 0;

    // .. PARA EL MISMO PORQUE ES LA RAIZ
    std::strcpy(nueva_ncarpeta.b_content[1].b_name, "..");
    nueva_ncarpeta.b_content[1].b_inodo = 0;

    //----
    // INODO UNO CON EL USER.TXT
    std::strcpy(nueva_ncarpeta.b_content[2].b_name, "users.txt");
    nueva_ncarpeta.b_content[2].b_inodo = 1;

    fseek(disk_file, nuevo.s_block_start, SEEK_SET);
    fwrite(&nueva_ncarpeta, sizeof(bcarpetas), 1, disk_file);

    // INODO 1 PARA EL ARCHIVO users.txt
    fseek(disk_file, nuevo.s_bm_inode_start + 1, SEEK_SET);
    fwrite(&uno, sizeof(char), 1, disk_file);

    // BLOQUE 1 ES DE TIPO ARCHIVO
    char a = 'a';
    fseek(disk_file, nuevo.s_bm_block_start + 1, SEEK_SET);
    fwrite(&a, sizeof(char), 1, disk_file);

    // DATA DEL INODO 1--ROOT, USUARIO Y PASSWORD
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
    nuevo_inodo.i_block[0] = 1;    // usa el bloque 1
    nuevo_inodo.i_type     = '1';  // ARCHIVO 1
    nuevo_inodo.i_perm     = 777;

    // SE AGREGA A LA TABLA
    fseek(disk_file, nuevo.s_inode_start + sizeof(inodo), SEEK_SET);
    fwrite(&nuevo_inodo, sizeof(inodo), 1, disk_file);

    // BLOQUE DE ARCHIVO PARA USERS.TXT
    barchivos nuevo_barchivo;
    std::strcpy(nuevo_barchivo.b_content, contenido.c_str());

    // BLOQUE 1
    fseek(disk_file, nuevo.s_block_start + sizeof(barchivos), SEEK_SET);
    fwrite(&nuevo_barchivo, sizeof(barchivos), 1, disk_file);


    //final
    /*
    superbloque
    bitmap de inodos- inodo 0 y 1: usados (1)
    bitmap de bloques- bloque 0 y 1 usados, bloque 0 de carpeta, bloque 1 de archivo
    --- INODO 0
    capeta raiz
    - bloque 0 -BCARPETA
    -- contenido, raiz y users.txt
    --- INODO 1
    archivo users.txt
    - bloque 1
    -- contenido del archivo - BARCHIVOS
    */

    std::cout << "[SUCCESS] La particion fue formateada correctamente." << std::endl;
    fclose(disk_file);
}
