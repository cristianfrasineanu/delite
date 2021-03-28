/**
 *  @brief Bitmap implementation file.
 *  
 */
 
#include "bitmap.h"

#ifdef USE_COLOR_TABLE
int BitmapInit8BitGrayscale(struct Bitmap **bitmap) {
    return EXIT_SUCCESS;
}

int BitmapGetColorTable(const struct Bitmap *bitmap,
                        union Raw_Pixel_Data **data) {
    return EXIT_SUCCESS;
}
#endif /* USE_COLOR_TABLE */

int BitmapSetWidthHeight(struct Bitmap *bitmap, 
                         uint32_t width,
                         uint32_t height) {
    return EXIT_SUCCESS;
}

uint32_t BitmapGetPixelData(const struct Bitmap *bitmap,
                            union Raw_Pixel_Data **data) {
    return 0;
}

int BitmapFillPixelData(struct Bitmap *bitmap, union Raw_Pixel_Data *data) {
    return EXIT_SUCCESS;
}

static int Scale16BitTo8Bit(uint16_t *in,
                            uint8_t **out, 
                            uint32_t count) {
    return EXIT_SUCCESS;
}