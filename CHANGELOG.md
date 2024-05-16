# Changelog

## 0.4.23

* remove ScanScope ID and Rack Tags metadata in Aperio format
* replace value of metadata Date, Time and Slide Tag with corresponding datatype instead of X's in Aperio format
* replace value of metadata DATETIME and Serial number with corresponding datatype instead of X's in Hamamatsu format
* replace value of metadata SLIDE_CREATIONDATETIME with corresponding datatype instead of x's in MIRAX format

## 0.4.22

* fix free of null and philips tiff short circuit
* remove obsolete templates

## 0.4.21

* fix segmentation faults and double frees for BIF/TIF format scanned by DP600
* extend testcases by DP 600 scanned file

## 0.4.20

* fix NDPI bug for files >5GB
* extend testcases

## 0.4.19

* update README

## 0.4.18

* remove values for refernce and scanner serial number tags for NDPI format

## 0.4.17

* fix MIRAX bug when anonymizing with WASM by checking result of wiped levels
* correctly extract file extension in WASM when working with MIRAX

## 0.4.16

* support anonymization for Aperio format scanned by KFBIO KF-PRO-400 and Motic Pro6

## 0.4.15

* fix double free or corrupted for Aperio GT450 format

## 0.4.14

* correctly overwrite image description in Aperio

## 0.4.13

* handle iSyntax Files bigger than 2GB in WASM

## 0.4.12

* allow import of shared library in python wrapper independent of current directory
* create DLL in MakefileWin for python wrapper

## 0.4.11

* replace check_file_header by get_wsi_data function
* extend testcases by new files
* remove all memory leaks (except for MIRAX)

## 0.4.10

* bugfix that correctly overwrites SLIDE_ID with random number in slidedat.ini and Data files for MIRAX format

## 0.4.9

* handle large (>4GB) NDPI files
* override metadata in NDPI files

## 0.4.8

* bugfix for wiping label data of ventana tif/bif files
* only first strip/tile war overwritten

## 0.4.7

* extended support for ventana dp600 format

## 0.4.6

* full Philips' TIFF support
* refactor philips based formats

## 0.4.5

* bugfix for string contains function in utils (replaced by string.h's `strstr` function)

## 0.4.4

* quickfix for mirax file structure bug in Slidedat.ini
* commented out closing of file handle that causes `corrupted size vs. prev_size` error in some cases

## 0.4.3

* bugfix for Aperio files with BigTiff structure
* remove date and barcode from metadata in aperio file

## 0.4.1 & 0.4.2

* bugfix for writing out mirax Slidedat.ini file
* bugfix of non hierarchical order in Slidedat.ini
* keep preview in mirax format

## 0.4.0

- change AnonymizedStream API for easier path mapping to support multi-anonymization of WSI files with same names

## 0.3.15

- add download option for MIRAX under WASM
- temporary disable unlinking option for .bif files
- remove TIFFTAG_DATETIME for .bif
- fix errors when anonymizing .isyntax under Ubuntu, Windows and WASM

## 0.3.14

- fix WASM bugs for ventana and isyntax
- allow passing of directories for MIRAX
- add download option for all file formats except MIRAX
- handle 32-Bit integer limitation

## 0.3.13

* enabled mirax unlinking

## 0.3.12

* temporary disabled mirax unlinking due to bug for some format versions
!! FIX AS SOON AS POSSIBLE !!

## 0.3.11

- refactor code
- separate source files by format
- implement expandable format approach

## 0.3.10

- updated README.md

## 0.3.9

- implement windows support for all data formats

## 0.3.8

- full isyntax support

## 0.3.7

- Remove ScanDataLayer_WholeSlide and ScanDataLayer_SlidePreview
- Remove more metadata

## 0.3.6

- Updated Readme

## 0.3.5

- Added ventana anonymization

## 0.3.4

- Added integration tests

## 0.3.3

- Remove metadata from tiff-based files

## 0.3.2

- Remove metadata from tiff-based files

## 0.3.1

- Basic support for Leica Aperio GT450
- Bugfix for Mirax
- Added code checks to CI

## 0.3.0

- Configurable chunk size

## 0.2.0

- Result propagation
- Fix filename and string handling

## 0.1.0

- Init
