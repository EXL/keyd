Fake App
========

Fake application, just a testing stub.

## Build

```bash
export QTDIR=/home/exl/Projects/qt2
cd fake_lib/
make clean
make
cd ../
make clean
make
```

## Run

```
LD_LIBRARY_PATH=/home/exl/Projects/qt2/src:`pwd`/fake_lib:$LD_LIBRARY_PATH ./FakeApp
```
