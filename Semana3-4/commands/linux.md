# Linux commands


## 1) `pwd` — *Print Working Directory* 

Muestra la ruta actual

```bash
pwd
```

---

## 2) `ls` — *List* 

lista archivos y carpetas, sirve para ver el contenido de un directorio.

```bash
ls
ls -l
ls -a
```

* -l: formato largo (permisos, tamaño, fecha).
* -a: incluye los que estan ocultos, empiezan con .

---

## 3) `cd` — *Change Directory* 

Moverte entre directorios 

```bash
cd commands
cd ..
cd /
cd ~
```

* ..: sube un nivel.
* /: raíz del sistema.
* ~: home

---

## 4) `mkdir` — *Make Directory*

Crea carpeta

```bash
mkdir commands
mkdir -p commands/test/test2
```

* -p: crea rutas completas aunque no existan.

---

## 5) `rmdir` — *Remove Directory* 

borra carpeta vacia

```bash
rmdir commands/test/test2
```

> Si no está vacía, se usa `rm -r` 

---

## 6) `touch` 


crear archivo vacío o actualizar fecha

```bash
touch ejemplos.txt
touch ejemplos.txt ejemplos2.txt
```

---

## 7) `cp` — *Copy* 
copiar archivos/carpetas

```bash
cp ejemplos.txt copia_ej.txt
cp -r carpeta_origen carpeta_copia
```

* -r: copia carpetas con todo su contenido.


---

## 8) `mv` — *Move* 

Mover o cambiar nombres

```bash
mv ejemplos.txt ejemplosNuevi.txt     # renombrar
mv ejemplos.txt /tmp/               # mover
```

---

## 9) `rm` — *Remove* 

Borrar

```bash
rm archivo.txt
rm -r carpeta
rm -i archivo.txt
```

* -r: borra carpetas con contenido
* -i: pregunta antes de borrar


---

## 10) cat 

mostrar contenido completo 

```bash
cat catTest.txt
```


---

## 11) head 
ver primeras líneas

```bash
head -n 5 catTest.txt
```

---

## 12) tail

ver últimas líneas

```bash
tail catTest.txt
tail -n 5 catTest.txt
```

---

## 13) echo — 

imprimir texto

```bash
echo "Hola"
echo "CLASE4" >catTest.txt

```
---

## 14) ps 
procesos en ejecución

```bash
ps
ps aux | head
```

---

## 15) top
 monitor de procesos en vivo

```bash
top
```

Salir: `q`
