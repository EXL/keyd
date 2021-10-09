libredkey
=========

A special library for hacking red key button behavior on MotoMAGX platform.

## Build

```bash
make clean
make
make tar # or
make zip
```

## Use

```bash
LD_PRELOAD=libredkey.so <some app> # or
export LD_PRELOAD=libredkey.so
./<some app>
```
