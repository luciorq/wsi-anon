# wsi-anon

C library to anonymize WSIs by remove the label image.

Currently supported formats:

* Aperio (`.svs` / `.tif`)
* Hamamatsu (`.ndpi`)
* Mirax (`.mrxs`)

## Prerequisites

* install build-essential

## Build

To build the console application simply run

```bash
make
```

This will build the object files and subsequently a static and a shared library. 
Also the console application will be build as .out file. These files are stored in `/bin/`.

To build the console application in debug mode type

```bash
make console-app-debug
```

and run with `gdb -args wsi-anon-dbg.out` afterwards.

## Tests

// to come

## Run

Check for slide vendor:

```bash
./wsi-anon.out "/path/to/wsi.tif" -c
```

Anonyimize slide:

```bash
./wsi-anon.out "/path/to/wsi.tif" [-OPTIONS]
```

Add `-h` for help. Further flags are:

* `-n "label-name"`: File will be renamed to given label name.
* `-u` : Disables the unlinking of tiff directories (default: dir will be unlinked)
* `-i` : Enable in-place anonymization. (default: copy of file will be created)
