#include "../include/image.h"
#include "../include/utils.h"
#include "../include/json_ops.h"

#include <string.h>
#include <stdlib.h>

void fill_png(libattopng_t *png, int x, int y, int *fill_color, int alpha) {
    for (int i = 0; i < x * y; ++i) {
        png->data[i * 4 + 0] = fill_color[0];  // Red
        png->data[i * 4 + 1] = fill_color[1];  // Green
        png->data[i * 4 + 2] = fill_color[2];  // Blue
        png->data[i * 4 + 3] = alpha;          // Alpha
    }
}

void set_pixels_from_json(libattopng_t *png, cJSON *json, int *fg, float highest_value){
    cJSON *current = NULL;

    cJSON_ArrayForEach(current, json) {
        if (strcmp(current->string, "config") == 0) continue;
        char key[MAX_KEY_LENGTH];
        strcpy(key, current->string);
        key[sizeof(key) - 1] = '\0';
        float current_value = get_value(json, key);
        char *x = strtok(key, "x");
        char *y = strtok(NULL, "x");
        libattopng_set_pixel(png, atoi(x), atoi(y), RGBA(
            (int)(current_value / highest_value * fg[0]),
            (int)(current_value / highest_value * fg[1]), 
            (int)(current_value / highest_value * fg[2]), 
            MAX_RGBA_VALUE));
    }
}

int generate_image(cJSON *json) {
    float highest_value = get_highest_value(json);
    int bg[3];
    int fg[3];
    int screen_x, screen_y;
    cJSON* config_json = get_config(json);
    int debug = get_value(config_json, "debug");
    
    screen_size(&screen_x, &screen_y);
    libattopng_t *png = libattopng_new(screen_x, screen_y, PNG_RGBA);
    
    if (debug) info("Image generation started");

    for(int i=0; i<3; i++){
        bg[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(config_json, "rgb_background"), i)->valueint;
        fg[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(config_json, "rgb_activity"), i)->valueint;
    }

    fill_png(png, screen_x, screen_y, bg, MAX_RGBA_VALUE);
    set_pixels_from_json(png, json, fg, highest_value);

    libattopng_save(png, PNG_FILENAME);
    libattopng_destroy(png);

    if (debug) info("Image generation ended");
    return 0;
}

