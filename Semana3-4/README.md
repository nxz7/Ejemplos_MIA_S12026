# Clase 3 y 4 - fdisk y mount

Se usa la base de la Clase 2 y se agregan los comandos:


- **fdisk**: crea particiones p/e/l
    STRUCT: MBR, EBR, LIBRE, POSITION, LIBREL

- **mount**: monta una partición y le asigna un id interno.


## test

```bash
cd Semana3-4
g++ -std=c++17 main.cpp analizador.cpp mkdisk.cpp rmdisk.cpp fdisk.cpp mount.cpp -o clase3
./clase3
```


```bash
mkdisk >size=5 >unit=m >fit=bf >path="/home/natalia/Documents/discos/Disco1.dsk"
fdisk >size=1024 >unit=k >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Part1 >type=p >fit=bf
fdisk >size=1 >unit=m >path="/home/natalia/Documents/discos/Disco1.dsk" >name=PartEXT2 >type=e >fit=ff
fdisk >size=1 >unit=m >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica5 >type=l >fit=wf
mount >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica

```
