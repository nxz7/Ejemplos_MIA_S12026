# Clase 8 -inodo

En esta clase se muestra como construir el reporte de inodos utilizados y generar un reporte en foma de lista, como se indica en el enuncias


## test

```bash
cd Semana8
g++ -std=c++17 main.cpp analizador.cpp mkdisk.cpp rmdisk.cpp fdisk.cpp mount.cpp mkfs.cpp \
    rep.cpp reportes/mbr.cpp reportes/disk.cpp reportes/inode.cpp  -o clase8
./clase8
```


```bash
mkdisk >size=5 >unit=m >fit=bf >path="/home/natalia/Documents/discos/Disco1.dsk"
fdisk >size=1024 >unit=k >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Part1 >type=p >fit=bf
fdisk >size=1 >unit=m >path="/home/natalia/Documents/discos/Disco1.dsk" >name=PartEXT2 >type=e >fit=ff
fdisk >size=100 >unit=k >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica >type=l >fit=wf
mount >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica
fdisk >size=120 >unit=k >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica2 >type=l >fit=ff
mkfs >id=0070Disco1 >type=full
mount >path="/home/natalia/Documents/discos/Disco1.dsk" >name=Partlogica2
mkfs >id=0071Disco1 >type=full
rep >id=0070Disco1 >path="/home/natalia/Documents/SEM9/Practicas/Ejemplos_MIA_S12026/Semana8/reportes/mbr.png" >name=mbr
rep >id=0070Disco1 >path="/home/natalia/Documents/SEM9/Practicas/Ejemplos_MIA_S12026/Semana8/reportes/disk.png" >name=disk
rep >id=0070Disco1 >path="/home/natalia/Documents/SEM9/Practicas/Ejemplos_MIA_S12026/Semana8/reportes/inode.png" >name=inode
```