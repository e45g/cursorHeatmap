
#include <string.h>

#include "../include/json_ops.h"
#include "../include/utils.h"
#include "../include/file_io.h"

int get_value(cJSON *json, char *key) {
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);
    return value ? value->valueint : 0;
}

long get_highest_value(cJSON *json) {
    cJSON *config = get_config();
    int debug = get_value(config, "debug");
    if (debug) info("get_highest_value called.");
    long highest = -1;
    cJSON *current = NULL;

    cJSON_ArrayForEach(current, json) {
        if (strcmp(current->string, "config") == 0) continue;
        int value = current->valueint;
        if (value > highest)
            highest = value;
    }
    cJSON_Delete(config);
    if (debug) info("get_highest_value finished.");
    return highest;
}

long unsigned filter_positions(cJSON *json){
    cJSON *config = get_config();
    cJSON *current = NULL;
    int treshold = get_value(config, "ignore_threshold");

    long removed = 0;
    long remove_count = 0;
    long json_size = cJSON_GetArraySize(json);
    char remove_list[json_size][MAX_KEY_LENGTH];

    float highest_value = get_highest_value(json);
    cJSON_ArrayForEach(current, json){
        if(strcmp(current->string, "config") == 0) continue;
        float value = current->valueint;
        if((value/(highest_value/100)) < treshold) {
            strncpy(remove_list[remove_count], current->string, MAX_KEY_LENGTH-1);
            remove_list[remove_count][MAX_KEY_LENGTH - 1] = '\0';
            remove_count++;
        }
    }

    for(int i = 0; i < remove_count; i++){
        cJSON_DeleteItemFromObject(json, remove_list[i]);
        removed++;
    }

    cJSON_Delete(config);
    return removed;
}

cJSON *generate_config() {
    cJSON *config_json = cJSON_CreateObject();

    cJSON_AddNumberToObject(config_json, "ignore_threshold", 1);
    cJSON_AddNumberToObject(config_json, "polling_rate", 1000);
    cJSON_AddNumberToObject(config_json, "debug", 0);
    cJSON_AddNumberToObject(config_json, "hidden", 0);
    cJSON_AddNumberToObject(config_json, "save_interval", 5);
    cJSON_AddItemToObject(config_json, "rgb_background", cJSON_CreateIntArray((const int[]){0, 0, 0}, 3));
    cJSON_AddItemToObject(config_json, "rgb_activity", cJSON_CreateIntArray((const int[]){MAX_RGBA_VALUE, MAX_RGBA_VALUE, MAX_RGBA_VALUE}, 3));

    return config_json;
}

cJSON *get_config() {
    cJSON *json = cJSON_CreateObject();
    if (load(DATA_FILENAME, &json) != 0) {
        cJSON_Delete(json);
        return NULL;
    }

    cJSON *config = cJSON_Duplicate(cJSON_GetObjectItem(json, "config"), 1);
    cJSON_Delete(json);
    return config;
}


