/**
 *  @brief Bitmap implementation file.
 *  
 */
 
#include "bitmap.h"

#ifdef USE_COLOR_TABLE
int BitmapInit8BitGrayscale(struct Bitmap **bitmap) {
    int status = EXIT_SUCCESS;
    struct Bitmap_ColorEntry *color_table = NULL;
    
    if (NULL == bitmap) {
        status = EXIT_FAILURE;
    }
    
    *bitmap = malloc(sizeof(struct Bitmap));
    
    return status;
}
#endif /* USE_COLOR_TABLE */

int BitmapSetWidthHeight(struct Bitmap *bitmap, 
                         uint32_t width,
                         uint32_t height) {
    return EXIT_SUCCESS;
}

uint32_t BitmapGetPixelData(const struct Bitmap *bitmap,
                            union Raw_Pixel_Data **data) {
    /* TODO (useful for reverse engineering images) */

    return 0;
}

int BitmapFillPixelData(struct Bitmap *bitmap, union Raw_Pixel_Data *data) {
    return EXIT_SUCCESS;
}
