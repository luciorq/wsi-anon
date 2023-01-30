# WSI Anon

## Description

A C library to anonymize Whole Slide Images in proprietary file formats. The library removes all sensitive data within the file structure including the filename itself, associated image data (as label and macro image) and metadata that is related to the slide acquisition and/or tissue. Associated image data is overwritten with either blank or white image data and subsequently unlinked from the file structure if possible. Unlinking can be disabled by a CLI / method parameter (for later pseudonymization). The related metadata is always removed from the file, usually containing identifiers or acquisition-related information as serial numbers, date and time, users, etc. A wrapper for JavaScript (WebAssembly) and python is provided.

Currently supported formats:

| Vendor | Scanner types (tested) | File extension | Comment |
|---|---|---|---|
| Leica Aperio | AT20, GT450 | `*.svs` `*.tif` | - |
| Hamamatsu | NanoZoomer XR, XT2, S360 | `*.ndpi` | - |
| 3DHistech Mirax | Pannoramic P150, P250, P1000 | `*.mrxs` | - |
| Roche Ventana | VS200, iScan Coreo | `*.bif` | - |
| Philips | IntelliSite Ultra Fast Scanner | `*.isyntax` | - |

The library is implemented and tested under Linux (Ubuntu 20.04). 

## Publications

The design and implementation is described in a technical note (currently as a preprint): [Anonymization of Whole Slide Images in Histopathology for Research and Education](https://arxiv.org/abs/2211.06103)

## Requirements

* install `build-essential`
* install `MinGW-w64`, e.g. from [Winlibs](https://winlibs.com/) (only required when running under windows)

WebAssembly:
* install `emscripten` (only required for Web Assembly target)

Development (Testing and code checks):
* install `clang-format-10`
* install `libcunit1-dev`
* install `docker` and `docker-compose`

## Build

### Native Target

#### Under Linux

To build the shared library with command line interface simply run

```bash
make
```

and run with `bin/wsi-anon.out /path/to/wsi.tif` afterwards.

This will build the object files and subsequently a static and a shared library. Also the console application will be build as .out file. These files are stored under `bin/`. Note that this will use the default Makefile.

To build the console application in debug mode type

```bash
make console-app-debug
```

and run with `gdb -args bin/wsi-anon-dbg.out /path/to/wsi.tif` afterwards.

#### Under Windows

To build an executable file under windows make sure `MinGW-w64` is installed and run

```bash
# if not yet done, set path for mingw after installation, e.g.:
set PATH=C:\mingw\bin;%PATH%

# compile
mingw32-make -f MakefileWin.mk
```

and run with `exe\wsi-anon.exe \path\to\wsi.tif` afterwards.

### Web Assembly Target

The library also has a Web Assembly (WASM) target in order to enable client-side anonymization of supported file formats from the browser. In this case the file I/O system calls are redirected to JavaScript and evaluated there for chunk reading and writing. **This is currently experimental.**

```bash
make wasm
```

This produces an ES6 module `./bin/wsi-anon.mjs` with embedded, base64 encoded wasm binary to facilitate usage in arbitrary web applications.

## Run

### Console Application

Check for slide vendor:

```bash
./wsi-anon.out "/path/to/wsi.tif" -c
```

Anonymize slide:

```bash
./wsi-anon.out "/path/to/wsi.tif" [-OPTIONS]
```

Type `-h` or `--help` for help. Further CLI parameters are:

* `-n "label-name"`: File will be renamed to the given label name
* `-u` : Disables the unlinking of associated image data (default: associated image will be unlinked)
* `-i` : Enable in-place anonymization (default: copy of the file will be created)

### Web Assembly Usage

In order to test the WASM build, you can use the [wasm-example.html](./wasm-example.html) page which contains a very basic integration of the generated ES6 module. This API - provided by a corresponding NPM package - can then also be imported from the given package:

```javascript
import AnonymizedStream from 'wsi-anon'
```

The `AnonymizedStream` class can be instantiated using its static create function and anoymized via its asynchronous anonymize method:

```javascript
const chunkSize = 10 * 1000*1000; // 10 MB
const stream = AnonymizedStream.create(file, chunkSize)
try {
  await stream.anonymize()
} catch (error) {
  console.error(error)
}
```

Anonymization is not done during creation of the instance, because there are WSI formats that consist of multiple files. For all of them, an `AnonymizedStream` instance must be created first. After that, `anonymize` is called **only** for the WSI's main file. After `anonymize` has been successfully awaited, all stream instances belonging to the given WSI can be uploaded, e.g., using the tus.Upload client.


## Development

### Add a Format

In order to extend this library by another format, please check the `plugin.h` header file and follow the steps as described.

### Code Formatting

Format the code before committing. Install `clang-format-10` and run

```
find . \( \( -name \*.c -o -name \*.h \) -a ! -iname \*soap\* \) -print0 | xargs -0 -n 1 clang-format-10 --Werror -i --verbose
```

### Unit Tests

To run unit tests install `libcunit1-dev` and build test projects with 

```bash
make tests
./bin/utests
```

### Integration Tests

To run integration tests install `docker` and `docker-compose`. Start the testing environment

```bash
cp sample.env .env
docker-compose -f docker-compose.test.yml up -d --build
```

and run tests with

```bash
docker exec wsi-anon_wsi-anon_1 pytest wrapper/python/test
```
