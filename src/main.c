/**
 *  @brief Application entry point
 *  
 *  This file contains the CLI entry point and 
 *  the pixel manipulation logic.
 *  
 */

#include "bitmap.h"

/*TODO: Add buffered logging functionality. */

/* System includes */
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/****************************************************************************
 * SYMBOLIC CONSTANTS
 ****************************************************************************/
/* Default path for the binary file containing the adjusted pixel data. */
#define ALTERED_FILE_PATH "altered.bin"

/****************************************************************************
 * LOCAL DECLARATIONS
 ****************************************************************************/
/**
 *  @brief Print help message
 *
 *  Print the help mesage for the app usage.
 *  @param none
 * 
 *  @return none
 */
static void PrintUsage(void);

/**
 *  @brief Check if a given file is regular through POSIX API.
 *
 *  @param file_path  File path to be checked
 * 
 *  @return true, if file is regular.
 *          false, otherwise.
 */
static bool FileIsRegular(const char *file_path);

/**
 *  @brief Process a raw pixel array by decreasing
 *         the gray intensity for overexposed pixels.
 * 
 *  The pixel array is traversed, and the highest pixel_count elements
 *  are adjusted by decreasing their value by adjustment_level%.
 *  @param data              Pixel data to adjust
 *  @param size              Array size
 *  @param pixel_count       Number of pixels to consider
 *  @param adjustment_level  Adjustment level (as a percentage)
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
static int AdjustPixelData(uint16_t *data, 
                           uint32_t size,
                           uint32_t pixel_count,
                           uint8_t adjustment_level);

/**
 *  @brief Read raw byte data from a file.
 * 
 *  A new array is allocated dependening on the size of the file,
 *  and the whole file is read into the newly allocated array.
 *  @param in    File to read from
 *  @param data  Generic reference to read into
 * 
 *  @return The size of the read data, if successful.
 *          0, otherwise.
 */
static uint32_t ReadBytesFromFile(FILE *in, void **data);

/**
 *  @brief Generate preview bitmap from a 16-bit encoded pixel array.
 * 
 *  The array is first scaled down to an 8-bit encoded array, then
 *  the bitmap is initialized and populated with the given pixel data. 
 *  @param data  Input pixel data
 *  @param size  Data size
 *  @param bmp   Output preview bitmap
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
static int GeneratePreviewBitmapFrom16Bit(const uint16_t *data, 
                                          uint32_t size,
                                          struct Bitmap **bmp);

/**
 *  @brief Write a variable to file byte-by-byte.
 * 
 *  Write count bytes of the given input to the specified file.
 *  @param out    Output file
 *  @param data   Memory to be written
 *  @param count  Number of bytes to write
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
static int WriteBytesToFile(FILE *out, const void *data, uint32_t count);

/**
 *  @brief Write a bitmap to file.
 * 
 *  @param out    Output file
 *  @param bm     Bitmap to write
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
static int WriteBmpToFile(FILE *out, const struct Bitmap *bmp);

/**
 *  @brief Run parameterized pixel adjustment.
 * 
 *  Read the input raw byte stream, detect overexposed pixels and
 *  output the altered binary file + the preview bitmap.
 *  @param input_file_path    Path to the input file 
 *  @param preview_file_path  Path to the final preview bitmap 
 *  @param pixel_count        Number of pixels to adjust
 *  @param adjustment_level   Adjustment level as percentage
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
static int RunAdjustment(const char *input_file_path,
                         const char *preview_file_path,
                         unsigned pixel_count,
                         unsigned adjustment_level);

/****************************************************************************/

/**
 *  @brief Main function
 * 
 *  Validate the user input received via CLI arguments, parse the arguments
 *  and run the adjustment algo.
 *  @param argc  Number of arguments
 *  @param argv  Reference to the argument array
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
int main (int argc, char **argv) {
    char **arg_iterator = NULL;
    char input_file_path[256] = { '\0' };
    char preview_file_path[256] = "out.bmp";
    unsigned pixel_count = 50U;
    unsigned adjustment_level = 50U;
    int status = EXIT_SUCCESS;

    /* Need at least one argument. */ 
    if (argc < 2) {
        PrintUsage();
        status = EXIT_FAILURE;
    }
    else {
        for (arg_iterator = argv + 1; NULL != *arg_iterator; arg_iterator++) {
            if (2 == strlen(arg_iterator[0])) {
                switch ((*arg_iterator)[1]) {
                    case 'h':
                        PrintUsage();
                        *(arg_iterator + 1) = NULL;

                        break;
                    /* Input pixel data path */
                    case 'f':
                        arg_iterator++;
                        if ((NULL != *arg_iterator) &&
                            (FileIsRegular(*arg_iterator))) {
                            strcpy(input_file_path, *arg_iterator);
                        }
                        else {
                            printf("Invalid input file path.\n");
                            status = EXIT_FAILURE;
                            *(arg_iterator + 1) = NULL;
                        }

                        break;
                    /* Pixel count */
                    case 'p':
                        arg_iterator++;
                        if (NULL != *arg_iterator) {
                            pixel_count = strtol(*arg_iterator, NULL, 0);
                        }
                        else {
                            pixel_count = 0;
                        }
                        if (pixel_count == 0) {
                            printf("Invalid pixel count.\n");
                            status = EXIT_FAILURE;
                            *(arg_iterator + 1) = NULL;
                        }

                        break;
                    /* Adjustment level */
                    case 'l':
                        arg_iterator++;
                        if (NULL != *arg_iterator) {
                            adjustment_level = strtol(*arg_iterator, NULL, 0);
                        }
                        else {
                            adjustment_level = UINT32_MAX;
                        }
                        if (adjustment_level > 100) {
                            printf("Invalid adjustment level "
                                   "(must be a valid percentage).\n");
                            status = EXIT_FAILURE;
                            *(arg_iterator + 1) = NULL;
                        }

                        break;
                    /* Output path */
                    case 'o':
                        arg_iterator++;
                        /* TODO: Add more sensible path checking logic. */
                        if (NULL != *arg_iterator) {
                            strcpy(preview_file_path, *arg_iterator);
                        }
                        else {
                            printf("Invalid output file path.\n");
                            status = EXIT_FAILURE;
                            *(arg_iterator + 1) = NULL;
                        }

                        break;
                    /* Invalid input */
                    default:
                        PrintUsage();
                        status = EXIT_FAILURE;
                        
                        break;
                }
            }
            else {
                /* Invalid input */
                PrintUsage();
                status = EXIT_FAILURE;
            }
        }

        if ((EXIT_SUCCESS == status) && (0 == strlen(input_file_path))) {
            printf("You must provide a valid input file path.\n");
            status = EXIT_FAILURE;
        }
        else if (EXIT_SUCCESS == status) {
            status = RunAdjustment(input_file_path, preview_file_path,
                                   pixel_count, adjustment_level);
        }
    }

    return status;
}

static void PrintUsage(void) {
    char help_message[] = "Usage: delite -h | "
                          "-f <input_file> [-p pixel_count] "
                          "[-l adjustment_level] [-o output_file]\n"
                          "\n"
                          "-h  Display help message\n"
                          "-f  Raw pixel data file (must be binary)\n"
                          "-p  The first number of pixels to adjust "
                          "for over exposure (default is 50)\n"
                          "-l  Adjustment level given as a percentage "
                          "(default is 50%)\n"
                          "-o  Output preview file as a result of the "
                          "adjustment (default is out.bmp)\n";

    printf("%s", help_message);
}

static bool FileIsRegular(const char *file_path) {
   struct stat file_stat;
   bool result = true;

   if (stat(file_path, &file_stat) < 0) {
       result = false;
   }

   return result;
}

/* TODO: Add support for other word sizes. */
static int AdjustPixelData(uint16_t *data, 
                           uint32_t size,
                           uint32_t pixel_count,
                           uint8_t adjustment_level) {
    unsigned i = 0;
    unsigned j = 0;
    uint16_t current_max = 0;
    uint16_t max_index = 0;
    int status = EXIT_SUCCESS;
    /* In this case, a selection algorithm won't be more efficient.
       Use flag array instead to avoid wasting memory. */
    bool *adjusted_flag = NULL;

    adjusted_flag = calloc(size, sizeof(bool));

    if ((NULL == data) || (NULL == adjusted_flag) || (0 == size)) {
        status = EXIT_FAILURE;
    }
    else {
        for (i = 0; i < pixel_count; i++) {
            current_max = 0;
            for (j = 0; j < size; j++) {
                if ((data[j] > current_max) && (!adjusted_flag[j])) {
                    current_max = data[j];
                    max_index = j;
                }
            }
            data[max_index] *= (1 - ((float) adjustment_level) / 100);
            adjusted_flag[max_index] = true;
        }

        free(adjusted_flag);
    } 

    return status;
}

static uint32_t ReadBytesFromFile(FILE *in, void **data) {
    uint32_t size = 0;
    
    if ((NULL != in) && (NULL != data)) {
        /* Get total size. */
        fseek(in, 0, SEEK_END);
        size = ftell(in);
        fseek(in, 0, SEEK_SET);

        *data = malloc(size);
        if (NULL != *data) {
            if (size != fread(*data, 1, size, in)) {
                /* File read failed. */
                size = 0;
                free(*data);
            }
        }
    }

    return size;
}

static int GeneratePreviewBitmapFrom16Bit(const uint16_t *data, 
                                          uint32_t size,
                                          struct Bitmap **bmp) {
    unsigned i = 0;
    unsigned image_size = 0;
    union Raw_Pixel_Data *scaled_data = NULL;
    int status = EXIT_SUCCESS;
    
    scaled_data = malloc(size * sizeof(union Raw_Pixel_Data));
    if ((NULL == data) || (scaled_data == NULL) 
        || (0 == size) || (NULL == bmp)) {
        status = EXIT_FAILURE;
    }
    
    status = BitmapInit8BitGrayscale(bmp);
    if (EXIT_SUCCESS == status) {
        image_size = sqrt(size);
        /* The width must be a multiple of 4. */
        image_size = image_size & ~0x03;
        /* Trim data size if the size isn't a perfect square. */
        size = image_size * image_size;
    
        for (i = 0; i < size; i++) {
            /* TODO: Average out the scaled data array
                     in case the size gets trimmed. */
            scaled_data[i].u8 = data[i] / 256U;
        }

        status = BitmapSetWidthHeight(*bmp, image_size, image_size);
        if (EXIT_SUCCESS == status) {
            status = BitmapFillPixelData(*bmp, scaled_data);
        }
    }
    
    return status;
}

static int WriteBytesToFile(FILE *out, const void *data, uint32_t count) {
    int status = EXIT_SUCCESS;
    
    if ((NULL != out) && (NULL != data)) {
        if (count != fwrite(data, 1U, count, out)) {
            status = EXIT_FAILURE;
        }
        else {
            fflush(out);
        }
    }
    else {
        status = EXIT_FAILURE;
    }

    return status;
}

static int WriteBmpToFile(FILE *out, const struct Bitmap *bmp) {
    int status = EXIT_SUCCESS;
    void *mem_to_write = NULL;
    
    if (NULL == bmp) {
        status = EXIT_FAILURE;
    }
    else {
        mem_to_write = malloc(GET_BITMAP_SIZE(bmp));
        if (NULL != mem_to_write) {
            memcpy(mem_to_write, &(bmp->header), sizeof(bmp->header));
            memcpy(mem_to_write + sizeof(bmp->header),
                   &(bmp->info_header),
                   bmp->info_header.header_size);
            memcpy(mem_to_write + sizeof(bmp->header) 
                   + bmp->info_header.header_size, 
                   bmp->color_table,
                   bmp->info_header.colors_used 
                   * sizeof(struct Bitmap_ColorEntry));
            memcpy(mem_to_write + bmp->header.pixel_data_offset,
                   bmp->pixel_data,
                   bmp->info_header.image_size);
            
            status = WriteBytesToFile(out, mem_to_write, GET_BITMAP_SIZE(bmp));
        }
        else {
            status = EXIT_FAILURE;
        }
    }

    return status;
}

static int RunAdjustment(const char *input_file_path,
                         const char *preview_file_path,
                         unsigned pixel_count,
                         unsigned adjustment_level) {
    struct Bitmap *output_bmp = NULL;
    FILE *in = NULL;
    FILE *out = NULL;
    uint16_t *raw_data = NULL;
    uint32_t raw_data_size = 0U;
    int status = EXIT_SUCCESS;

    in = fopen(input_file_path, "rb");
    raw_data_size = ReadBytesFromFile(in, (void **) &raw_data);

    /* TODO: Add dedicated error reporting. */ 
    if (raw_data_size > 0) {
        status = AdjustPixelData(raw_data,
                                 raw_data_size / sizeof(raw_data[0]),
                                 pixel_count,
                                 adjustment_level);
        if (EXIT_SUCCESS == status) {
            /* Output the adjusted pixel data in binary. */
            out = fopen(ALTERED_FILE_PATH, "wb");
            status = WriteBytesToFile(out, raw_data, raw_data_size);
            if (EXIT_SUCCESS == status) {
                fclose(out);
                status = GeneratePreviewBitmapFrom16Bit(raw_data, 
                                                        raw_data_size /
                                                        sizeof(raw_data[0]),
                                                        &output_bmp);
                if (EXIT_SUCCESS == status) {
                    out = fopen(preview_file_path, "wb");
                    status = WriteBmpToFile(out, output_bmp);
                    if (EXIT_FAILURE == status) {
                        printf("Unexpected error when writing the "
                               "preview bitmap.\n");
                    }

                    free(output_bmp->color_table);
                    free(output_bmp->pixel_data);
                }
                else {
                    printf("Unexpected error when generating the preview.\n");
                }
            }
            else {
                printf("Unexpected error when writing the "
                       "adjusted pixel data to file.\n");
            }
        }
        else {
            printf("Unexpected error when processing the pixel data.\n");
        }

    }
    else {
        printf("Unexpected error when reading the raw input byte stream.\n");
        status = EXIT_FAILURE;
    }

    if (NULL != in) {
        fclose(in);
    }
    if (NULL != out) {
        fclose(out);
    }

    /* Free tolerates NULL, no need to check. */
    free(raw_data);
    free(output_bmp);

    return status;
}