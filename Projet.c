#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

// Convertir un octet en couleur RGB
// 0-85: BLEU (texte ASCII), 86-170: VERT (données moyennes), 171-255: ROUGE (binaire pur)
void byte_to_color(unsigned char byte, unsigned char *r, unsigned char *g, unsigned char *b) {
    if (byte <= 85) {
        *r = 0;
        *g = byte;
        *b = 85 + (byte * 2);
    } else if (byte <= 170) {
        int pos = byte - 86;
        *r = pos;
        *g = 170 + pos;
        *b = 170 - pos;
    } else {
        int pos = byte - 171;
        *r = 171 + pos;
        *g = 255 - pos;
        *b = 0;
    }
}

// Extraire le nom sans le chemin (ex: "/bin/bash" -> "bash")
const char *extract_filename(const char *path) {
    const char *name = strrchr(path, '/');
    if (!name) {
        name = strrchr(path, '\\');  // Windows
    }
    return name ? name + 1 : path;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        printf("Utilisez --help pour plus d'informations\n");
        return 1;
    }

    // Gérer --help
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        printf("Usage: %s <fichier_binaire>\n", argv[0]);
        printf("\n");
        printf("Convertit un fichier binaire en image PNG.\n");
        printf("Chaque octet est représenté par un pixel coloré.\n");
        printf("\n");
        printf("Options:\n");
        printf("  -h, --help    Affiche cette aide\n");
        printf("\n");
        printf("Exemple:\n");
        printf("  %s /bin/bash    → crée bash.png\n", argv[0]);
        return 0;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Error: file not found\n");
        return 1;
    }

    // Obtenir la taille via fseek/ftell
    fseek(f, 0, SEEK_END);
    long size_tmp = ftell(f);
    rewind(f);

    unsigned char *data = malloc(size_tmp);
    if (!data) {
        printf("Error: not enough memory\n");
        fclose(f);
        return 1;
    }
    size_t size = fread(data, 1, size_tmp, f);
    fclose(f);

    // Calcul pour obtenir une image carrée
    int width = 1;
    while (width * width < (int)size) {
        width++;
    }
    int height = ((int)size + width - 1) / width;

    // Créer nom de sortie dynamique
    const char *input_name = extract_filename(argv[1]);
    size_t output_name_len = strlen(input_name) + 5;
    char *output_name = malloc(output_name_len);
    if (!output_name) {
        printf("Error: not enough memory for filename\n");
        free(data);
        return 1;
    }
    snprintf(output_name, output_name_len, "%s.png", input_name);

    FILE *fp = fopen(output_name, "wb");
    if (!fp) {
        printf("Error: cannot create %s\n", output_name);
        free(output_name);
        free(data);
        return 1;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        printf("Error: PNG creation failed\n");
        fclose(fp);
        free(output_name);
        free(data);
        return 1;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        printf("Error: PNG info creation failed\n");
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        free(output_name);
        free(data);
        return 1;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    unsigned char *row = malloc(width * 3);
    if (!row) {
        printf("Error: not enough memory for row\n");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        free(output_name);
        free(data);
        return 1;
    }

    size_t idx = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char val = (idx < size) ? data[idx++] : 0;
            unsigned char r, g, b;
            byte_to_color(val, &r, &g, &b);
            row[x * 3 + 0] = r;
            row[x * 3 + 1] = g;
            row[x * 3 + 2] = b;
        }
        png_write_row(png, row);
    }

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    free(row);
    free(data);

    printf("Image created: %s\n", output_name);
    free(output_name);
    return 0;
}
