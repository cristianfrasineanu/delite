/**
 *  @brief Bitmap format definition header.
 *  
 *  This header contains bitmap-related types definitions,
 *  including the bitmap manipulation API declaration.
 *  The functions can be used for initializing bitmaps and performing
 *  read/write operations on the pixel data.
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdlib.h>

/****************************************************************************
 * SYMBOLIC CONSTANTS
 ****************************************************************************/
/* Bitmap magic sequence, corresponding to 'BM' in ASCII. */
#define BITMAP_MAGIC 0x424DU

#ifndef USE_COLOR_TABLE
    #warning "Support for monochromatic, 4-bit and 8-bit bitmaps is disabled!"
#endif

/****************************************************************************
 * TYPE DEFINITIONS
 ****************************************************************************/

/* Set the structure members alignment to 1,
   so the file layout matches exactly. */
#pragma pack(push)
#pragma pack(1)

/**
 *  @brief Application identification header.
 */
struct Bitmap_Header {
    const uint16_t signature;   /* Magic Bytes sequence 
                                   (for .bmp must be "BM") */
    uint32_t file_size;         /* The total file size in bytes
                                   (header+info header+color table+data) */
    const uint32_t reserved;    /* Not used */
    uint32_t pixel_data_offest; /* The offset to the actual pixel data */
};

/**
 *  @brief Bitmap specific information.
 */ 
struct Bitmap_Info_Header {
    const uint32_t header_size;       /* The current header size */
    uint32_t width;                   /* Bitmap width 
                                        (must be a multiple of 4 bytes) */
    uint32_t height;                  /* Bitmap height */
    const uint16_t planes_count;      /* Number of planes (=1) */
    const uint16_t bit_depth;         /* Bit depth level (up to 24-bit) */
    const uint32_t compression;       /* Compression type */
    uint32_t image_size;              /* Image size after compression
                                         (if uncompressed, == width * height) */
    uint32_t x_resolution;            /* Horizontal resolution (pixels/meter) */
    uint32_t y_resolution;            /* Vertical resolution (pixels/meter) */
    const uint32_t colors_used;       /* Number of colors 
                                         (e.g 0x100 for 8-bit) */
    const uint32_t important_colors;  /* Number of important colors
                                         (0 for all) */
};

/** 
 *  @brief Pixel data acessing word.
*/
union Raw_Pixel_Data {
    uint8_t u8;                 /* 1-byte word */
    uint16_t u16;               /* 2-byte word */
};

#ifdef USE_COLOR_TABLE
/** 
 *  @brief Color table enty containing each channel's intensity.
*/
struct Bitmap_ColorEntry {
    uint8_t red;                /* Red intensity */
    uint8_t green;              /* Green intensity */
    uint8_t blue;               /* Blue intensity */
    uint8_t reserved;           /* Not used */
};
#endif /* USE_COLOR_TABLE */

/** 
 *  @brief Bitmap structure resembling an actual file.
 * 
 *  The color table size is determined based on the bit depth
 *  E.g. for an 8-bit depth image, we'll have 256 entries.
 *  The pixel data array size is computed based on the
 *  image size contained by the info_header.
*/
struct Bitmap {
    struct Bitmap_Header header;
    struct Bitmap_Info_Header info_header;
#ifdef USE_COLOR_TABLE
    struct Bitmap_ColorEntry *color_table;
#endif /* USE_COLOR_TABLE */
    union Raw_Pixel_Data *pixel_data;
};

/* Restore the initial memory alignment. */
#pragma pack(pop)

/****************************************************************************
 * FUNCTION DECLARATIONS
 ****************************************************************************/

#ifdef USE_COLOR_TABLE
/**
 *  @brief Initialize an 8-bit grayscale bitmap.
 *
 *  Initialize the main bitmap fields, having the default 
 *  grayscale color table alongside (256 shades).
 *  @param bitmap  Bitmap to be initialized
 * 
 *  @return EXIT_SUCCESSFUL, if successful.
 *          EXIT_FAILURE, if not successful.
 */
int BitmapInit8BitGrayscale(struct Bitmap **bitmap);

/**
 *  @brief Get the bitmap color table.
 *
 *  Copy the color table entries to a 
 *  @param bitmap  Bitmap to read from
 *  @param table   Reference to the beginning of the color table
 * 
 *  @return The size of the color table array, if successful.
 *          -1, if not successful.
 */
int BitmapGetColorTable(const struct Bitmap *bitmap,
                        union Raw_Pixel_Data **data);
#endif /* USE_COLOR_TABLE */

/**
 *  @brief Set the width and height of the bitmap.
 *
 *  Update the width and height and, implicitly, the image_size.
 *  The function will fail if the width is not a multiple of 4 bytes.
 *  (see DIB format specs)
 *  @param bitmap  Bitmap to be modified
 *  @param width   Image width
 *  @param heigth  Image height
 * 
 *  @return EXIT_SUCCESSFUL, if successful.
 *          EXIT_FAILURE, if not successful.
 */
int BitmapSetWidthHeight(struct Bitmap *bitmap, 
                         uint32_t width,
                         uint32_t height);

/**
 *  @brief Get the raw pixel data from a bitmap.
 *
 *  Determine the pixel data location to read from by parsing the 
 *  information header of the bitmap, and then change the reference
 *  to the beginning of the pixel array.
 *  @param bitmap  Bitmap to read from
 *  @param data    Reference to the beginning of the array
 * 
 *  @return The size of the pixel data array, if successful.
 *          0, if not successful.
 */
uint32_t BitmapGetPixelData(const struct Bitmap *bitmap,
                            union Raw_Pixel_Data **data);

/**
 *  @brief Copy the input data to the bitmap pixel data array.
 *
 *  Copy each byte from the input data array to the given bitmap.
 *  The input data array is assumed to have at least image_size bytes,
 *  otherwise the behavior is undefined.
 *  @param bitmap  Bitmap to be modified
 *  @param data    Array containing the raw pixel values
 * 
 *  @return EXIT_SUCCESSFUL, if successful.
 *          EXIT_FAILURE, if not successful.
 */
int BitmapFillPixelData(struct Bitmap *bitmap, union Raw_Pixel_Data *data);

/**
 *  @brief Scale down a 16-bit encoded array of pixels to 8-bit format.
 * 
 *  The input pixel data gets converted to an 8-bit encoded grayscale image
 *  by dividing each item by 256 (for each 8-bit gray shade,
 *  there are 256 shades in 16-bit format).
 *  @param in     Input pixel data
 *  @param out    Scaled pixel data
 *  @param count  Input elements count
 * 
 *  @return EXIT_SUCCESS, if successful
 *          EXIT_FAILURE, otherwise.
 */
static int Scale16BitTo8Bit(uint16_t *in,
                            uint8_t **out, 
                            uint32_t count);

/****************************************************************************/

#endif /* BITMAP_H */