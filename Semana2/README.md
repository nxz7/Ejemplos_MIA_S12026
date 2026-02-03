# Clase 2 - MKDISK y RMDISK
En esta clase se tomo la base de la clase anterior y se agregaron los comandos mkdisk y rmdisk. Estos comandos permiten crear y eliminar archivos binarios, que simularan los discos

- **mkdisk**: crea el archivo del disco. 
**obligatorios:**
* size 
* path

**opcionales:** 
* unit - k/m 
* fit - bf/ff/wf

- **rmdisk**: elimina el dico
**obligatorios:** 
* path

## TEST

```bash
cd Clase2
g++ -std=c++17 main.cpp analizador.cpp mkdisk.cpp rmdisk.cpp -o clase2
./clase2
```

```bash
mkdisk >size=5 >unit=m >fit=bf >path="/home/natalia/Documents/discos/Disco1.dsk"
mkdisk >size=277 >unit=k >fit=ff >path="/home/natalia/Documents/discos/Disco7.dsk"
rmdisk >path="/home/natalia/Documents/discos/Disco1.dsk"
```
