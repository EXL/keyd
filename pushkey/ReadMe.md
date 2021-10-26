pushkey
=======

A special utility for emulating pressing keypad buttons on Motorola MotoMAGX platform.

## Build

```bash
make clean
make
make tar
make zip
```

## Use

```bash
pushkey <some keycode>

pushkey 0x80000001 # (push 1)
pushkey 0x00000001 # (release 1)

pushkey 0x80000001 server # (push 1)
pushkey 0x00000001 server # (release 1)
```
