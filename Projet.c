#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

int main(int argc, char *argv[]) {
    // Check arguments: we need a filename
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // Open and read the entire binary file
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Error: file not found\n");
        return 1;
    }

    // Get file size by seeking to end
    fseek(f, 0, SEEK_END);
    long size_tmp = ftell(f);
    rewind(f);

    unsigned char *data = malloc(size_tmp);
    size_t size = fread(data, 1, size_tmp, f);
    fclose(f);

    // Calculate square-ish image dimensions
    int width = (int)ceil(sqrt(size));
    int height = (int)ceil((double)size / width);

    // Setup PNG file with libpng
    FILE *fp = fopen("output.png", "wb");
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, fp);

    // Configure: RGB format, 8 bits per channel
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // Write pixels: one row at a time
    unsigned char *row = malloc(width * 3);  // 3 bytes per pixel (RGB)
    size_t idx = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char val = (idx < size) ? data[idx++] : 0;
            // Grayscale: same value for R, G, B
            row[x * 3 + 0] = val;
            row[x * 3 + 1] = val;
            row[x * 3 + 2] = val;
        }
        png_write_row(png, row);
    }

    // Cleanup
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    free(row);
    free(data);

    printf("Image created: output.png\n");
    return 0;
}
