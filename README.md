# wsi-anon

C library to anonymize WSIs by label image removal

Currently supported formats:

* Aperio (`.svs` / `.tif`)
* Hamamatsu (`.ndpi`)
* Mirax (`.mrxs`)

## Prerequisites

* install build-essential
* install emscripten (only required for Web Assembly target)

## Build

### Native Target

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

### Web Assembly Target

The library also has a Web Assembly (WASM) target in order to enable client-side anonymization
of supported file formats from the browser. In this case the file I/O system calls are redirected
to JavaScript and evaluated there for chunk reading and writing. **This is currently experimental.**

```bash
make wasm
```

This produces an ES6 module `./bin/wsi-anon.mjs` with embedded, base64 encoded wasm binary to facilitate usage in arbitrary web applications.

## Tests

To run unit tests build project with 

```bash
make tests
./bin/utests
```

// to come: integration tests

## Run

### Console Application

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
