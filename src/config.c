#include "../include/config.h"
#include "../include/utils.h"

#include <string.h>

int get_value(cJSON *json, char *key) {
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);
    return value ? value->valueint : 0;
}

long get_highest_value(cJSON *json) {
    int debug = get_value(get_config(json), "debug");
    if (debug) info("get_highest_value called.");
    long highest = -1;
    cJSON *current = NULL;

    cJSON_ArrayForEach(current, json) {
        if (strcmp(current->string, "config") == 0)
            continue;
        int value = current->valueint;
        if (value > highest)
            highest = value;
    }
    if (debug) info("get_highest_value finished.");
    return highest;
}

cJSON *generate_config(cJSON *json) {
    cJSON *config_json = cJSON_CreateObject();

    cJSON_AddNumberToObject(config_json, "polling_rate", 1000);
    cJSON_AddNumberToObject(config_json, "debug", 0);
    cJSON_AddNumberToObject(config_json, "hidden", 0);
    cJSON_AddNumberToObject(config_json, "save_interval", 5);
    cJSON_AddItemToObject(config_json, "rgb_background", cJSON_CreateIntArray((const int[]){0, 0, 0}, 3));
    cJSON_AddItemToObject(config_json, "rgb_activity", cJSON_CreateIntArray((const int[]){0, MAX_RGBA_VALUE, 0}, 3));

    cJSON_AddItemToObject(json, "config", config_json);
    return config_json;
}

cJSON *get_config(cJSON *json) {
    return cJSON_GetObjectItem(json, "config");
}
