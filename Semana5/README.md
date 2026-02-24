# Clase 5 - MKFS

Se usa la base de la Clase 3-4 y se agregan los comandos:


- **mkfs**: formatea la particion montada segun el ID, se genera el superbloque, bitmaps,inodos, bloques


## test

```bash
cd Semana5
g++ -std=c++17 main.cpp analizador.cpp mkdisk.cpp rmdisk.cpp fdisk.cpp mount.cpp mkfs.cpp -o clase5
./clase5
```

```bash
mkdisk >size=5 >unit=m >fit=bf >path="/home/natalia/Documents/discos/Disco1.dsk"
fdisk >size=1024 >unit=k >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Part1 >type=p >fit=bf
fdisk >size=1 >unit=m >path="/home/natalia/Documents/discos/Disco1.dsk" >name=PartEXT2 >type=e >fit=ff
fdisk >size=100 >unit=k >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica >type=l >fit=wf
mount >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica
mkfs >id=0070Disco1 >type=full
```
