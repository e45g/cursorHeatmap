#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/image.h"
#include "../include/utils.h"
#include "../include/json_ops.h"
#include "../include/file_io.h"

void fill_png(libattopng_t *png, int x, int y, int *fill_color, int alpha) {
    for (int i = 0; i < x * y; ++i) {
        png->data[i * 4 + 0] = fill_color[0];  // Red
        png->data[i * 4 + 1] = fill_color[1];  // Green
        png->data[i * 4 + 2] = fill_color[2];  // Blue
        png->data[i * 4 + 3] = alpha;          // Alpha
    }
}

void set_pixels_from_json(libattopng_t *png, cJSON *json, int *fg, float highest_value) {
    cJSON *current = NULL;

    int pixel_count = 0;
    PixelData *pixel_data = NULL;

    cJSON_ArrayForEach(current, json) {
        if (strcmp(current->string, "config") != 0) {
            pixel_count++;
        }
    }

    pixel_data = (PixelData *)malloc(pixel_count * sizeof(PixelData));
    if (!pixel_data) {
        return;
    }

    int index = 0;
    cJSON_ArrayForEach(current, json) {
        if (strcmp(current->string, "config") == 0) continue;

        char *key = current->string;
        char *x_str = strtok(key, "x");
        char *y_str = strtok(NULL, "x");

        if (x_str && y_str) {
            pixel_data[index].x = atoi(x_str);
            pixel_data[index].y = atoi(y_str);
            pixel_data[index].value = (float)cJSON_GetNumberValue(current);
            index++;
        }
    }

    for (int i = 0; i < index; i++) {
        float value_ratio = pixel_data[i].value / highest_value;
        int r = (int)(value_ratio * fg[0]);
        int g = (int)(value_ratio * fg[1]);
        int b = (int)(value_ratio * fg[2]);

        libattopng_set_pixel(png, pixel_data[i].x, pixel_data[i].y, RGBA(r, g, b, MAX_RGBA_VALUE));
    }

    free(pixel_data);
}


int generate_image() {
    cJSON *json = cJSON_CreateObject();
    load(DATA_FILENAME, &json);
    float highest_value = get_highest_value(json);
    int bg[3];
    int fg[3];
    int screen_x, screen_y;
    cJSON* config = get_config();
    int debug = get_value(config, "debug");
    screen_size(&screen_x, &screen_y);
    libattopng_t *png = libattopng_new(screen_x, screen_y, PNG_RGBA);
    
    if (debug) info("Image generation started");

    for(int i=0; i<3; i++){
        bg[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(config, "rgb_background"), i)->valueint;
        fg[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(config, "rgb_activity"), i)->valueint;
    }

    fill_png(png, screen_x, screen_y, bg, MAX_RGBA_VALUE);
    set_pixels_from_json(png, json, fg, highest_value);

    libattopng_save(png, PNG_FILENAME);
    libattopng_destroy(png);

    cJSON_Delete(json);
    cJSON_Delete(config);

    if (debug) info("Image generation ended");
    return 0;
}

void *save_and_generate_image(void *json) {
    cJSON *config = get_config();
    int sleep = cJSON_GetObjectItemCaseSensitive(config, "save_interval")->valueint;
    cJSON_Delete(config);
    while (1) {
        lock_json();
        save(json);
        generate_image();
        unlock_json();
        usleep(sleep * 60 * 1000 * 1000);
    }
}

