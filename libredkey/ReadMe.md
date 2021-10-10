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

## Additional

```bash
LD_PRELOAD=libredkey.so LIBREDKEY_CONFIG_PATH=/mmc/mmca1/libredkey.cfg ./<some app>
LD_PRELOAD=libredkey_no_config.so LIBREDKEY_ON=1 ./<some app>
```
