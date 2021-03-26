# Delite

Delite is a lightweight exposure detector utlity for adjusting overexposed images by reducing the overall brightness.

Currently, there is support only for **16-bit greyscale** rasters in .bmp format. This might change in the future.

## Prerequisites

- GNU Make
- A standard C compiler (e.g gcc, clang etc.) which supports the C99 standard (i.e. ISO/IEC 9899:1999)

## Usage

### Building the project

```shell
git clone https://github.com/cristianfrasineanu/delite.git
cd $_
make
```
After executing these commands, the final executable will reside in the bin/ folder.

If you want to have it available globally, run `make install`.

### CLI usage

Flag | Details
---- | ----
-h | Display help message
-f | Input bitmap file (must be a .bmp)
-p | The first number of pixels, in terms of intensity, to adjust (default is 50)
-l | Adjustment level given as a percentage (default is 25%)
-o | Output bitmap file as a result of the adjustment

Example:
Detect overexposed pixels by turning the first 50 pixels which have the highest value black:

```shell
delite -f rsc/demo.bmp -l 100 -o adjusted.bmp 
```

## Licensing

[GPLv3](https://choosealicense.com/licenses/gpl-3.0/)
