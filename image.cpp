#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "image.h"
#include "macro.h"
#include "maths.h"

/* image creating/releasing */

image_t *image_create(int width, int height, int channels)
{
    int buffer_size = width * height * channels;
    image_t *image;

    assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);

    image = (image_t*)malloc(sizeof(image_t));
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->buffer = (unsigned char*)malloc(buffer_size);
    memset(image->buffer, 0, buffer_size);
    return image;
}

void image_release(image_t *image)
{
    free(image->buffer);
    free(image);
}

/* image input/output */

static int get_buffer_size(image_t *image)
{
    return image->width * image->height * image->channels;
}

static unsigned char read_byte(FILE *file)
{
    int byte = fgetc(file);
    assert(byte != EOF);
    return (unsigned char)byte;
}

static void read_bytes(FILE *file, void *buffer, int size)
{
    int count = (int)fread(buffer, 1, size, file);
    assert(count == size);
    UNUSED_VAR(count);
}

static void write_bytes(FILE *file, void *buffer, int size)
{
    int count = (int)fwrite(buffer, 1, size, file);
    assert(count == size);
    UNUSED_VAR(count);
}

static void load_tga_rle(FILE *file, image_t *image)
{
    unsigned char *buffer = image->buffer;
    int channels = image->channels;
    int buffer_size = get_buffer_size(image);
    int elem_count = 0;
    while (elem_count < buffer_size) {
        unsigned char header = read_byte(file);
        int rle_packet = header & 0x80;
        int pixel_count = (header & 0x7F) + 1;
        unsigned char pixel[4];
        int i, j;
        assert(elem_count + pixel_count * channels <= buffer_size);
        if (rle_packet) {  /* rle packet */
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[elem_count++] = pixel[j];
                }
            }
        } else {           /* raw packet */
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[elem_count++] = read_byte(file);
                }
            }
        }
    }
    assert(elem_count == buffer_size);
}

#define TGA_HEADER_SIZE 18

static image_t *load_tga(const char *filename)
{
    unsigned char header[TGA_HEADER_SIZE];
    int width, height, depth, channels;
    int idlength, imgtype, imgdesc;
    image_t *image;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_bytes(file, header, TGA_HEADER_SIZE);

    width = header[12] | (header[13] << 8);
    height = header[14] | (header[15] << 8);
    assert(width > 0 && height > 0);
    depth = header[16];
    assert(depth == 8 || depth == 24 || depth == 32);
    channels = depth / 8;
    image = image_create(width, height, channels);

    idlength = header[0];
    assert(idlength == 0);
    UNUSED_VAR(idlength);
    imgtype = header[2];
    if (imgtype == 2 || imgtype == 3) {           /* uncompressed */
        read_bytes(file, image->buffer, get_buffer_size(image));
    } else if (imgtype == 10 || imgtype == 11) {  /* run-length encoded */
        load_tga_rle(file, image);
    } else {
        assert(0);
    }
    fclose(file);

    imgdesc = header[17];
    if (imgdesc & 0x20) {
        image_flip_v(image);
    }
    if (imgdesc & 0x10) {
        image_flip_h(image);
    }
    return image;
}

static void save_tga(image_t *image, const char *filename)
{
    unsigned char header[TGA_HEADER_SIZE];
    FILE *file;

    file = fopen(filename, "wb");
    assert(file != NULL);

    memset(header, 0, TGA_HEADER_SIZE);
    header[2] = image->channels == 1 ? 3 : 2;     /* image type */
    header[12] = image->width & 0xFF;             /* width, lsb */
    header[13] = (image->width >> 8) & 0xFF;      /* width, msb */
    header[14] = image->height & 0xFF;            /* height, lsb */
    header[15] = (image->height >> 8) & 0xFF;     /* height, msb */
    header[16] = (image->channels * 8) & 0xFF;    /* image depth */
    write_bytes(file, header, TGA_HEADER_SIZE);

    write_bytes(file, image->buffer, get_buffer_size(image));
    fclose(file);
}

static const char *extract_extension(const char *filename)
{
    const char *dot_pos = strrchr(filename, '.');
    return dot_pos == NULL ? "" : dot_pos + 1;
}

image_t *image_load(const char *filename)
{
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        return load_tga(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void image_save(image_t *image, const char *filename)
{
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        save_tga(image, filename);
    } else {
        assert(0);
    }
}

/* image processing */

static void swap_bytes(unsigned char *a, unsigned char *b)
{
    unsigned char t = *a;
    *a = *b;
    *b = t;
}

static unsigned char *get_pixel_ptr(image_t *image, int row, int col)
{
    int index = row * image->width * image->channels + col * image->channels;
    return &(image->buffer[index]);
}

void image_flip_h(image_t *image)
{
    int half_width = image->width / 2;
    int r, c, k;
    for (r = 0; r < image->height; r++) {
        for (c = 0; c < half_width; c++) {
            int flipped_c = image->width - 1 - c;
            unsigned char *pixel1 = get_pixel_ptr(image, r, c);
            unsigned char *pixel2 = get_pixel_ptr(image, r, flipped_c);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

void image_flip_v(image_t *image)
{
    int half_height = image->height / 2;
    int r, c, k;
    for (r = 0; r < half_height; r++) {
        for (c = 0; c < image->width; c++) {
            int flipped_r = image->height - 1 - r;
            unsigned char *pixel1 = get_pixel_ptr(image, r, c);
            unsigned char *pixel2 = get_pixel_ptr(image, flipped_r, c);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

image_t *image_resize(image_t *source, int width, int height)
{
    int channels = source->channels;
    image_t *target;
    float scale_r, scale_c;
    int dst_r, dst_c, k;

    assert(width > 0 && height > 0);
    target = image_create(width, height, channels);
    scale_r = (float)source->height / (float)height;
    scale_c = (float)source->width / (float)width;
    for (dst_r = 0; dst_r < height; dst_r++) {
        for (dst_c = 0; dst_c < width; dst_c++) {
            float mapped_r = (float)dst_r * scale_r;
            float mapped_c = (float)dst_c * scale_c;
            int src_r0 = (int)mapped_r;
            int src_c0 = (int)mapped_c;
            int src_r1 = int_min(src_r0 + 1, source->height - 1);
            int src_c1 = int_min(src_c0 + 1, source->width - 1);
            float delta_r = mapped_r - (float)src_r0;
            float delta_c = mapped_c - (float)src_c0;

            unsigned char *pixel_00 = get_pixel_ptr(source, src_r0, src_c0);
            unsigned char *pixel_01 = get_pixel_ptr(source, src_r0, src_c1);
            unsigned char *pixel_10 = get_pixel_ptr(source, src_r1, src_c0);
            unsigned char *pixel_11 = get_pixel_ptr(source, src_r1, src_c1);
            unsigned char *pixel = get_pixel_ptr(target, dst_r, dst_c);
            for (k = 0; k < channels; k++) {
                float v00 = pixel_00[k];  /* row 0, col 0 */
                float v01 = pixel_01[k];  /* row 0, col 1 */
                float v10 = pixel_10[k];  /* row 1, col 0 */
                float v11 = pixel_11[k];  /* row 1, col 1 */
                float v0 = float_lerp(v00, v01, delta_c);  /* row 0 */
                float v1 = float_lerp(v10, v11, delta_c);  /* row 1 */
                float value = float_lerp(v0, v1, delta_r);
                pixel[k] = (unsigned char)(value + 0.5f);
            }
        }
    }
    return target;
}
