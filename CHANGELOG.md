# Changelog

## 0.4.1

* bugfix for writing out mirax Slidedat.ini file

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
