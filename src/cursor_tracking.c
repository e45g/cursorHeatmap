#include "../include/cursor_tracking.h"
#include "../include/json_ops.h"
#include "../include/utils.h"

#include <stdlib.h>
#include <unistd.h>
#include <windows.h>
#include <stdio.h>

void process_position(cJSON *json, char *key) {
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (value != NULL && cJSON_IsNumber(value)) {
        cJSON_SetNumberValue(value, cJSON_GetNumberValue(value) + 1);
    } else {
        cJSON_AddNumberToObject(json, key, 1);
    }
}

void *position_logic(void *json) {
    POINT *coords = malloc(sizeof(POINT));
    cJSON *config = get_config(json);
    int last_coords[2] = {0, 0};
    int sleep_time = get_value(config, "polling_rate");

    while (1) {
        usleep(sleep_time);
        GetCursorPos(coords);

        char *pos = malloc(sizeof(char) * 20);
        sprintf(pos, "%ldx%ld", coords->x, coords->y);

        if (last_coords[0] != coords->x || last_coords[1] != coords->y) {
            lock_json();
            process_position(json, pos);
            unlock_json();
        }
        free(pos);

        last_coords[0] = coords->x;
        last_coords[1] = coords->y;
    }
    free(coords);
}