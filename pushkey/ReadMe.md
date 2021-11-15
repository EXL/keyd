pushkey
=======

A special utility for emulating pressing keypad buttons on Motorola MotoMAGX platform.

## Build for MotoMAGX device

```bash
make clean
make
make tar
make zip
```

## Build for MotoMAGX emulator

```bash
make -f Makefile.emu clean
make -f Makefile.emu
make -f Makefile.emu tar
make -f Makefile.emu zip
```

## Use

```bash
pushkey <keycode> <device> <mode>

pushkey 0x8001         # push "1" button on keypadI, default mode
pushkey 0x0001         # release "1" button on keypadI, default mode

pushkey 0x8001 i       # push "1" button on keypadI, default mode
pushkey 0x0001 i       # release "1" button on keypadI, default mode

pushkey 0x8001 b       # push "1" button on keypadB, default mode
pushkey 0x0001 b       # release "1" button on keypadB, default mode

pushkey 0x31 i a       # push "1" button on keypadI, ascii mode
pushkey 0x31 b a       # push "1" button on keypadB, ascii mode

pushkey 0x0001 i s     # push "1" button on keypadI, single mode
pushkey 0x0001 b s     # push "1" button on keypadB, single mode

pushkey 0x80000004 v   # push "4" button on vkm, default mode
pushkey 0x00000004 v   # release "4" button on vkm, default mode

pushkey 0x00000004 v s # push "4" button on vkm, single mode
```

## Key holding

```bash
for i in $(seq 1 10); do ./pushkey_EMU 0x80000004 v; usleep 100000; done; usleep 100000; ./pushkey_EMU 0x00000004 v

./pushkey 0x8036 i; sleep 1; ./pushkey 0x0019 i s; ./pushkey 0x0036 i
```

## Additional Information

See ["Эмуляция нажатия клавиш на Motorola ZN5"](https://forum.motofan.ru/index.php?s=&showtopic=163337&view=findpost&p=1794896) post (in Russian) on the [MotoFan.Ru](https://forum.motofan.ru) forum.
