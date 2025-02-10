#include <stdlib.h>
#include <string.h>

#include "image.h"

uint8_t create_bmp(Bitmap *dst, uint8_t *src) {
    int offset = 0;
    dst->img = NULL;

    // Copy in the bitmap file header
    memcpy(dst->file_header, src, BMP_FILE_HEADER_SIZE);
    offset += BMP_FILE_HEADER_SIZE;

    // Get filesize
    dst->file_size = (dst->file_header[5] << 8 * 3) | (dst->file_header[4] << 8 * 2) |
                     (dst->file_header[3] << 8 * 1) | (dst->file_header[2]);

    // Get start location of pixel data
    dst->pxl_data_offset = (dst->file_header[13] << 8 * 3) | (dst->file_header[12] << 8 * 2) |
                           (dst->file_header[11] << 8 * 1) | (dst->file_header[10]);

    // // Get DIB header data
    uint8_t dib_header_size = dst->pxl_data_offset - BMP_FILE_HEADER_SIZE;
    dst->dib_header = (uint8_t *)malloc((dib_header_size) * sizeof(uint8_t));
    memcpy(dst->dib_header, src + offset, dib_header_size);
    offset += dib_header_size;

    // Get image width
    dst->img_width = (dst->dib_header[7] << 8 * 3) | (dst->dib_header[6] << 8 * 2) |
                     (dst->dib_header[5] << 8 * 1) | (dst->dib_header[4]);

    // Get image height
    dst->img_height = (dst->dib_header[11] << 8 * 3) | (dst->dib_header[10] << 8 * 2) |
                      (dst->dib_header[9] << 8 * 1) | (dst->dib_header[8]);

    if (dst->img_height < 0) {
        dst->img_height *= -1;
    }

    // Get image data
    dst->pxl_data_size = dst->img_width * dst->img_height * 3;
    dst->pxl_data = malloc(sizeof(uint8_t) * dst->pxl_data_size);
    memcpy(dst->pxl_data, src + offset, dst->pxl_data_size);

    // Create copy of image data for reset
    dst->pxl_data_cpy = malloc(sizeof(uint8_t) * dst->pxl_data_size);
    memcpy(dst->pxl_data_cpy, dst->pxl_data, dst->pxl_data_size);

    return LOAD_SUCCESS;
}

void destroy_bmp(Bitmap *bmp) {
    free(bmp->dib_header);
    free(bmp->pxl_data);
    free(bmp->pxl_data_cpy);
}

void reset_pixel_data(Bitmap *bmp) {
    bmp->pxl_data = memcpy(bmp->pxl_data, bmp->pxl_data_cpy, bmp->pxl_data_size);
}

uint8_t *get_pxl_data(Bitmap *bmp) { return bmp->pxl_data; }

uint8_t *get_original_pxl_data(Bitmap *bmp) { return bmp->pxl_data_cpy; }

void remove_color_channel(Color color, Bitmap *bmp) {
    // Put code here
    printf("remove_color_channel is executing ... \n");

    // variable color is 0(blue) 1(green) or 2(red)
    // go through the bmp array which contains the ints that form the pixels

    for (uint32_t y = 0; y < bmp->pxl_data_size; y += 3) {
        // uint32_t color_i_want = bmp->pxl_data[y * (3 + color)];

        uint32_t remove_this = y + color; // 3*y+color;

        bmp->pxl_data[remove_this] = 0;
    }
}

void or_filter(Bitmap *bmp) {
    // Put code here
    printf("or_filter is executing ...\n");

    // if top or bottom row:
    // take the two rows before or after it right?
    // bitwise or function
    // if not on top or bottom, take color value above and below it and
    // bitwise or them together so bitwising three things?
    // get data from pxl copy but change pxl_data

    for (uint32_t y = 0; y < bmp->pxl_data_size; y += 3) {

        uint32_t blue_pos = y + 0;
        uint32_t green_pos = y + 1;
        uint32_t red_pos = y + 2;

        // if first or last row
        if ((y <= (bmp->img_width * 3)) || (y >= bmp->pxl_data_size - (bmp->img_width * 3))) {
            // for now set pizel equal to zero
            bmp->pxl_data[blue_pos] = bmp->pxl_data[blue_pos];
            bmp->pxl_data[green_pos] = bmp->pxl_data[green_pos];
            bmp->pxl_data[red_pos] = bmp->pxl_data[red_pos];
        } else {
            // value we're focused on
            uint32_t blue_value = bmp->pxl_data_cpy[blue_pos];
            uint32_t green_value = bmp->pxl_data_cpy[green_pos];
            uint32_t red_value = bmp->pxl_data_cpy[red_pos];

            // value directly below
            uint32_t under_blue_value = bmp->pxl_data_cpy[blue_pos - bmp->img_width * 3];
            uint32_t under_green_value = bmp->pxl_data_cpy[green_pos - bmp->img_width * 3];
            uint32_t under_red_value = bmp->pxl_data_cpy[red_pos - bmp->img_width * 3];

            // value directly above
            uint32_t above_blue_value = bmp->pxl_data_cpy[blue_pos + bmp->img_width * 3];
            uint32_t above_green_value = bmp->pxl_data_cpy[green_pos + bmp->img_width * 3];
            uint32_t above_red_value = bmp->pxl_data_cpy[red_pos + bmp->img_width * 3];

            // bitwise addition of all three and sum should be stored in new_blue, etc
            uint32_t new_blue_value = blue_value | under_blue_value | above_blue_value;
            uint32_t new_green_value = green_value | under_green_value | above_green_value;
            uint32_t new_red_value = red_value | under_red_value | above_red_value;

            // changed pixels to the new colors
            bmp->pxl_data[blue_pos] = new_blue_value;
            bmp->pxl_data[green_pos] = new_green_value;
            bmp->pxl_data[red_pos] = new_red_value;
        }
    }
}