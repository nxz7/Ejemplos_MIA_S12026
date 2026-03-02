# Clase 6-7 ==== Diagrama MBR y disk

En base a los comandos generados en las clases anteriores, en esta clase se explico el codigo para generar dos de los reportes solicitados.
- MBR detalles sobre las particiones
- Disk detalles sobre el disco, general


## test




```bash
sudo apt update
sudo apt install graphviz
dot -V


cd Semana6-7
g++ -std=c++17 main.cpp analizador.cpp mkdisk.cpp rmdisk.cpp fdisk.cpp mount.cpp mkfs.cpp \
    rep.cpp reportes/mbr.cpp reportes/disk.cpp  -o clase6
./clase6
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
rep >id=0070Disco1 >path="/home/natalia/Documents/reportes/mbr.png" >name=mbr
rep >id=0070Disco1 >path="/home/natalia/Documents/reportes/disk.png" >name=disk

```