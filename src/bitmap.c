/**
 *  @brief Bitmap implementation file.
 *  
 */
 
#include "bitmap.h"
 
#include <string.h>

#ifdef USE_COLOR_TABLE
int BitmapInit8BitGrayscale(struct Bitmap **bitmap) {
    int status = EXIT_SUCCESS;
    unsigned i = 0U;
    struct Bitmap_ColorEntry *color_table = NULL;
    struct Bitmap_Header header = {
        .signature = BITMAP_MAGIC,
        .pixel_data_offset = sizeof(struct Bitmap_Header) +
                             sizeof(struct Bitmap_Info_Header) + 256U * 
                             sizeof(struct Bitmap_ColorEntry)
    };
    struct Bitmap_Info_Header info_header = {
        .header_size = sizeof(struct Bitmap_Info_Header),
        .planes_count = 1U,
        .bit_depth = 8U,
        /* TODO: Add compression support */
        .compression = 0,
        .colors_used = 256U,
    };
    
    if (NULL == bitmap) {
        status = EXIT_FAILURE;
    }
    
    *bitmap = malloc(sizeof(struct Bitmap));
    (*bitmap)->color_table = malloc(256U * sizeof(struct Bitmap_ColorEntry));
    color_table = (*bitmap)->color_table;

    if ((NULL == (*bitmap)) ||
        (NULL == color_table)) {
        status = EXIT_FAILURE;
    }
    else {
        /* Populate only static fields */
        memcpy((*bitmap), &header, sizeof(header));
        memcpy(&((*bitmap)->info_header), &info_header, sizeof(info_header));

        /* Create color table containing the 8-bit gray color palette. */
        for (i = 0; i < 256; i++) {
            color_table[i].red = i;
            color_table[i].green = i;
            color_table[i].blue = i;
            color_table[i].reserved = 0;
        }
    }
    
    return status;
}
#endif /* USE_COLOR_TABLE */

int BitmapSetWidthHeight(struct Bitmap *bitmap, 
                         uint16_t width,
                         uint16_t height) {
    int status = EXIT_SUCCESS;
    
    if (NULL == bitmap) {
        status = EXIT_FAILURE;
    }
    else {
        if (width % 4 != 0) {
            status = EXIT_FAILURE;
        }
        else {
            bitmap->info_header.image_size = width * height;
            bitmap->info_header.width = width;
            bitmap->info_header.height = height;
            bitmap->header.file_size = width * height + 
                                       bitmap->header.pixel_data_offset;
        }
    }

    return status;
}

uint32_t BitmapGetPixelData(const struct Bitmap *bitmap,
                            union Raw_Pixel_Data **data) {
    /* TODO */

    return 0;
}

int BitmapFillPixelData(struct Bitmap *bitmap, union Raw_Pixel_Data *data) {
    uint32_t i = 0;
    int status = EXIT_SUCCESS;

    if ((NULL == bitmap) || (NULL == data)) {
       status = EXIT_FAILURE; 
    }
    else {
        bitmap->pixel_data = malloc(bitmap->info_header.image_size);

        if (NULL == bitmap->pixel_data) {
            status = EXIT_FAILURE;
        }
        else {
            for (i = 0; i < bitmap->info_header.image_size; i++) {
                if (8U == bitmap->info_header.bit_depth) {
                    ((uint8_t *) bitmap->pixel_data)[i] = data[i].u8;
                }
                else {
                    /* TODO */
                    status = EXIT_FAILURE;
                }
            }
        }
    }
    
    return status;
}
