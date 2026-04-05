# Clase 11 - Frontend React + Backend + Journaling (EXT3)

En esta clase se agrega:

1. EL sistema EXT3 en mkfs con espacio de journaling.
2. Comando journaling para ver registros.
3. Backend  en Node/Express para ejecutar comandos del analizador
4. Frontend en React 



##  Compilar analizador 

```bash
cd Semana11
g++ -std=c++17 main.cpp analizador.cpp mkdisk.cpp rmdisk.cpp fdisk.cpp mount.cpp mkfs.cpp rep.cpp journaling.cpp \
    reportes/mbr.cpp reportes/disk.cpp reportes/inode.cpp reportes/bm_inode.cpp reportes/block.cpp -o clase11
```

## backend

```bash
cd Semana11/backend
npm install
npm run dev
```

Backend por defecto: `http://localhost:3001`

## frontend

```bash
cd Semana11/frontend
npm install
npm run dev
```

Frontend por defecto: `http://localhost:5173`



##  Comandos de prueba 

```bash
mkdisk >size=5 >unit=m >fit=bf >path="/tmp/Disco11.dsk"
fdisk >size=1024 >unit=k >path="/tmp/Disco11.dsk" >name=Part1 >type=p >fit=bf
mount >path="/tmp/Disco11.dsk" >name=Part1
mkfs >id=0070Disco11 >type=full >fs=3fs
rep >id=0070Disco11 >path="/tmp/block.png" >name=block
rep >id=0070Disco11 >path="/tmp/mbr.png" >name=mbr
journaling >id=0070Disco11
```



