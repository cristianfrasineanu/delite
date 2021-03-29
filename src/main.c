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
 *  @param count Number of pixels to consider
 *  @param bmp   Output preview bitmap
 * 
 *  @return EXIT_SUCCESS, if successful.
 *          EXIT_FAILURE, otherwise.
 */
static int GeneratePreviewBitmapFrom16Bit(const uint16_t *data, 
                                          uint32_t count,
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

static int AdjustPixelData(uint16_t *data, 
                           uint32_t size,
                           uint32_t pixel_count,
                           uint8_t adjustment_level) {
    int status = EXIT_SUCCESS;
    unsigned i = 0U;
    unsigned j = 0U;
    uint16_t previous_max = UINT16_MAX;
    uint16_t current_max = 0U;

    if ((NULL == data) || (0 == size)) {
        status = EXIT_FAILURE;
    }
    else {
        for (i = 0; i < pixel_count; i++) {
            current_max = 0U;
            for (j = 0; j < size; j++) {
                if ((data[j] > current_max) && (data[j] <= previous_max)) {
                    current_max = data[j];
                }
            }
            data[j] *= 1 - ((float) adjustment_level) / 100;
            previous_max = current_max;
        }
    } 

    return status;
}

static uint32_t ReadBytesFromFile(FILE *in, void **data) {
    uint32_t size = 0U;
    
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
                                          uint32_t count,
                                          struct Bitmap **bmp) {
    return EXIT_SUCCESS;
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

    return status;
}

static int WriteBmpToFile(FILE *out, const struct Bitmap *bmp) {
    return EXIT_SUCCESS;
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
    if (raw_data_size > 0U) {
        status = AdjustPixelData(raw_data,
                                 raw_data_size,
                                 pixel_count,
                                 adjustment_level);
        if (EXIT_SUCCESS == status) {
            /* Output the adjusted pixel data in binary. */
            out = fopen(ALTERED_FILE_PATH, "wb");
            status = WriteBytesToFile(out, raw_data, raw_data_size);
            if (EXIT_SUCCESS == status) {
                status = GeneratePreviewBitmapFrom16Bit(raw_data, 
                                                        raw_data_size,
                                                        &output_bmp);
                if (EXIT_SUCCESS == status) {
                    fclose(out);
                    out = fopen(preview_file_path, "wb");
                    status = WriteBmpToFile(out, output_bmp);
                    if (EXIT_FAILURE == status) {
                        printf("Unexpected error when writing the "
                               "preview bitmap.\n");
                    }
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

    fclose(in);
    fclose(out);
    /* Free tolerates NULL, no need to check. */
    free(raw_data);
    free(output_bmp);

    return status;
}