#ifndef IMAGE_H
#define IMAGE_H

#include "../lib/libattopng.h"
#include "../lib/cJSON.h"

void fill_png(libattopng_t *png, int x, int y, int *fill_color, int alpha);
void set_pixels_from_json(libattopng_t *png, cJSON *json, int *fg, float highest_value);
int generate_image();
void *save_and_generate_image(void *json);

#endif