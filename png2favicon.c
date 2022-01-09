// SPDX-License-Identifier: MIT
// Copyright © 2022 Anton Fors

#ifdef _WIN32
    // Do not throw deprecation warning for fopen on Windows
    #define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Takes pointer to byte array and converts to long, big-endian
#define B2L_BE(p) ((uint32_t)(*(p)) << 24 | (uint32_t)(*((p) + 1)) << 16 | (uint32_t)(*((p) + 2)) << 8 | (uint32_t)(*((p) + 3)))

// Utility macros to split shorts and longs to bytes, little-endian
#define S2B(s) (uint8_t)(s), (uint8_t)((s) >> 8)
#define L2B(l) (uint8_t)(l), (uint8_t)((l) >> 8), (uint8_t)((l) >> 16), (uint8_t)((l) >> 24)

// Only allow inclusion of PNG files up to 1 MiB in size
#define MAX_FILE_SIZE 1048576
#define MIN_FILE_SIZE 64

// ICO format related constants
#define ICONDIR_LENGHT 6
#define ICONDIRENTRY_LENGTH 16

const uint8_t PNG_HEADER[] = {
    // PNG signature
    137, 'P', 'N', 'G', 13, 10, 26, 10,
    // Start of IHDR chunk
    0, 0, 0, 13, 'I', 'H', 'D', 'R'
};

uint32_t read_png(char *filename, uint8_t **buffer, uint8_t *resolution) {
    // Open file for reading
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Could not open file: %s\n", filename);
        return 0;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    if (size <= MAX_FILE_SIZE && size >= MIN_FILE_SIZE) {
        // Reset file position
        rewind(file);

        // Read file into buffer
        *buffer = malloc(size);
        fread(*buffer, 1, size, file);

        if (memcmp(*buffer, PNG_HEADER, sizeof(PNG_HEADER)) == 0) {
            uint32_t width = B2L_BE(*buffer + 16);
            uint32_t height = B2L_BE(*buffer + 20);

            if (width == height && width >= 1 && width <= 256) {
                *resolution = (uint8_t)width;
            } else {
                printf("Illegal image resolution: %s\n", filename);
                free(*buffer);
                size = 0;
            }
        } else {
            printf("Not a PNG file: %s\n", filename);
            free(*buffer);
            size = 0;
        }
    } else {
        printf("Illegal file size: %s\n", filename);
        size = 0;
    }

    fclose(file);

    return (uint32_t)size;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 257) {
        printf(
            "Combines PNG files into a favicon.ico file.\n\n"
            "Usage: %s [files.png]...\n",
            argv[0]
        );
        return 0;
    }

    uint16_t image_count = argc - 1;

    uint8_t *buffers[image_count];
    uint8_t resolutions[image_count];
    uint32_t sizes[image_count];

    for (uint16_t i = 0; i < image_count; i++) {
        sizes[i] = read_png(argv[i + 1], &buffers[i], &resolutions[i]);

        if (sizes[i] == 0) {
            puts("Failed to generate a favicon.ico file, exiting.");
            while (i) free(buffers[--i]);
            return 1;
        }
    }

    FILE *file = fopen("favicon.ico", "wb");
    if (file == NULL) {
        puts("Could not open favicon.ico for writing, exiting.");
        return 1;
    }

    // ICONDIR
    fwrite((uint8_t[]) {
        S2B(0),             // Reserved, must be 0
        S2B(1),             // Image type, 1 for ICO
        S2B(image_count)    // Number of images
    }, 1, ICONDIR_LENGHT, file);

    uint32_t offset = ICONDIR_LENGHT + ICONDIRENTRY_LENGTH * image_count;

    for (uint16_t i = 0; i < image_count; i++) {
        // ICONDIRENTRY
        fwrite((uint8_t[]) {
            resolutions[i], // Image width
            resolutions[i], // Image height
            0,              // Number of colors in palette
            0,              // Reserved, should be 0
            S2B(0),         // Color planes, should be 0 or 1
            S2B(0),         // Bits per pixel, if 0 it will be inferred from PNG data
            L2B(sizes[i]),  // Image size
            L2B(offset)     // Offset in file
        }, 1, ICONDIRENTRY_LENGTH, file);

        offset += sizes[i];
    }

    for (uint16_t i = 0; i < image_count; i++) {
        fwrite(buffers[i], 1, sizes[i], file);
        free(buffers[i]);
    }

    fclose(file);

    puts("Successfully generated a favicon.ico file.");

    return 0;
}
