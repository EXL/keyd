libredkey
=========

A special library for hacking red key button behavior on MotoMAGX platform.

## Build

```bash
make clean
make
make tar
make zip
```

## Use

Put `libredkey.cfg` to the `/ezxlocal/download/appwrite/setup/` directory.

```bash
LD_PRELOAD=libredkey.so ./<some application>

export LD_PRELOAD=libredkey.so
./<some application>

LD_PRELOAD=libredkey.so LIBREDKEY_CONFIG_PATH=/mmc/mmca1/libredkey.cfg ./<some application>

export LD_PRELOAD=libredkey.so
export LIBREDKEY_CONFIG_PATH=/mmc/mmca1/libredkey.cfg
./<some application>

LIBREDKEY_ON=1 LD_PRELOAD=libredkey_no_config.so ./<some application>

export LD_PRELOAD=libredkey_no_config.so
export LIBREDKEY_ON=1
./<some application>
```
