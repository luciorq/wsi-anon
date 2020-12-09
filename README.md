# wsi-anon

C library to remove the label from WSIs.

## Prerequisites

* install libtiff-dev (ubuntu): will be removed shortly
* install cmake

## Build

To build the console application simply run

```bash
make console-wsi-anonymizer.a
```

## Run

Check for slide vendor:

```bash
./console-wsi-anonymizer.a -c //path//to//wsi.tif
```

Anonyimize slide:

```bash
./console-wsi-anonymizer.a -i //path//to//wsi.tif -n "new label name" -u
```

The -u flag specifies if a directory should be unlinked from the WSI.
