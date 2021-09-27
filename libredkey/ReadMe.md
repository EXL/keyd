libredkey.so
============

A special library for hacking red key button behavior on MotoMAGX platform.

## Build library for MotoMAGX

```bash
unset QTDIR
make clean
make
make tar
```

## Use library on MotoMAGX

```bash
LD_PRELOAD=libredkey.so <some app>
```

## Build testing application

```bash
export QTDIR=/home/exl/Projects/qt2
cd test_app/test_lib/
make clean
make
cd ..
make clean
make
cd ..
make -f Makefile.test clean
make -f Makefile.test
```

## Run testing application

```bash
LD_PRELOAD=libredkey.so LD_LIBRARY_PATH=/home/exl/Projects/qt2/src:`pwd`/test_app/test_lib:$LD_LIBRARY_PATH ./test_app/TestApp
```
