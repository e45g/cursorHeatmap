#include "../include/file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/cursor_tracking.h"
#include "../include/image.h"
#include "../include/json_ops.h"
#include "../include/utils.h"

int save(cJSON *json) {
    cJSON *saved_json = cJSON_CreateObject();
    cJSON *current = NULL;
    load(DATA_FILENAME, &saved_json);
    cJSON *config = cJSON_GetObjectItem(saved_json, "config");
    int debug = get_value(config, "debug");
    if (debug) info("saving started");

    long remove_count = 0;
    int json_size = cJSON_GetArraySize(json);
    char remove_list[json_size][MAX_KEY_LENGTH];

    cJSON_ArrayForEach(current, json) {
        if (strcmp(current->string, "config") != 0) {
            process_position(saved_json, current->string, current->valueint);
            strncpy(remove_list[remove_count], current->string, MAX_KEY_LENGTH - 1);
            remove_list[remove_count][MAX_KEY_LENGTH - 1] = '\0';
            remove_count++;
        }
    }

    for (int i = 0; i < remove_count; i++) {
        cJSON_DeleteItemFromObject(json, remove_list[i]);
    }

    FILE *fp = fopen(DATA_FILENAME, "w");
    if (!fp) {
        error("Unable to open the file.");
        cJSON_Delete(saved_json);
        return 1;
    }

    char *json_str = cJSON_Print(saved_json);
    if (!json_str) {
        fclose(fp);
        error("Error: `save`; cJSON_Print failed.");
        cJSON_Delete(saved_json);
        return 1;
    }

    if (fputs(json_str, fp) == EOF) {
        fclose(fp);
        cJSON_free(json_str);
        error("Error: `save`; Error writing to the file.");
        cJSON_Delete(saved_json);
        return 1;
    }
    fclose(fp);

    cJSON_free(json_str);
    cJSON_Delete(saved_json);
    if (debug) info("saved");
    return 0;
}

int create_data_file(const char *filename, cJSON *config_json) {
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        error("Error: `createInitialConfigFile`; Error creating %s", filename);
        fclose(fp);
        return 1;
    }
    char *config_str = cJSON_Print(config_json);
    if (config_str == NULL) {
        error("Error: `create_data_file`; Error converting cJSON to string");
        fclose(fp);
        return 1;
    }
    if (fprintf(fp, "{\"config\":%s}", config_str) < 0) {
        error("Error: `create_data_file`; Error writing to %s", filename);
        fclose(fp);
        return 1;
    }
    fclose(fp);
    cJSON_free(config_str);
    return 0;
}

char *read_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        error("Error: `read_file`; Error opening %s", filename);
        return NULL;
    }
    long file_size = get_file_size(fp);
    if (file_size < 0) {
        fprintf(stderr, "Error: `read_file`; Error getting file size\n");
        fclose(fp);
        return NULL;
    }

    char *buffer = (char *)malloc(sizeof(char) * (file_size + 1));
    if (buffer == NULL) {
        error("Error: `read_file`; Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, file_size, fp);
    if (read_size != (size_t)file_size) {
        error("Error: `read_file`; Error reading file %lld %ld\n\n%s", read_size, file_size, buffer);
        free(buffer);
        fclose(fp);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(fp);
    return buffer;
}

int load(const char *filename, cJSON **json) {
    cJSON *config_json = generate_config();

    if (access(filename, F_OK) == -1) {
        create_data_file(filename, config_json);
    }

    char *buffer = read_file(filename);
    if (buffer == NULL) {
        error("Error: `load`; Buffer is null");
        cJSON_Delete(config_json);
        return 1;
    }
    cJSON *parsed_json = cJSON_Parse(buffer);
    free(buffer);
    if (parsed_json == NULL) {
        error("Error: `load`; Error parsing JSON");
        cJSON_Delete(config_json);
        return 1;
    }
    *json = parsed_json;

    if (!cJSON_HasObjectItem(*json, "config")) {
        cJSON_AddItemToObject(*json, "config", config_json);
    } else {
        cJSON_Delete(config_json);
    }
    return 0;
}
