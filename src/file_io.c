#include "../include/file_io.h"
#include "../include/utils.h"
#include "../include/json_ops.h"
#include "../include/image.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int save(char *json_str) {
    cJSON *json = cJSON_Parse(json_str);
    int debug = get_value(get_config(json), "debug");
    if (debug) info("saving started");

    FILE *fp = fopen(DATA_FILENAME, "w");
    
    if (access(DATA_FILENAME, F_OK) == -1) {
        error("Unable to open the file.");
        exit(1);
    }
    if (fputs(json_str, fp) == EOF) {
        error("Error: `save`; Error writing to the file.");
        exit(1);
    }
    fclose(fp);

    if (debug) info("saved");
    cJSON_Delete(json);
    return 0;
}

void create_data_file(const char *filename, cJSON *config_json) {
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        error("Error: `createInitialConfigFile`; Error creating %s", filename);
        exit(1);
    }
    char *config_str = cJSON_Print(config_json);
    if (config_str == NULL) {
        error("Error: `create_data_file`; Error converting cJSON to string");
        exit(1);
    }
    if (fprintf(fp, "{\"config\":%s}", config_str) < 0) {
        error("Error: `create_data_file`; Error writing to %s", filename);
        exit(1);
    }
    fclose(fp);
    free(config_str);
}

char *read_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        error("Error: `read_file`; Error opening %s", filename);
        exit(1);
    }
    long file_size = get_file_size(fp);

    char *buffer = (char *)malloc(sizeof(char) * (file_size + 1));
    if (buffer == NULL) {
        error("Error: `read_file`; Memory allocation failed");
        fclose(fp);
        exit(1);
    }

    fread(buffer, 1, file_size, fp);
    buffer[file_size] = '\0';
    fclose(fp);
    return buffer;
}

int load(const char *filename, cJSON **json) {
    int debug = get_value(get_config(*json), "debug");
    if (debug) info("loading started.");

    cJSON *config_json = generate_config(*json);
    FILE *fp = fopen(filename, "r");

    if (access(filename, F_OK) == -1) {
        if (debug)
            info("creating %s", filename);
        create_data_file(filename, config_json);
    }

    char *buffer = read_file(filename);
    *json = cJSON_Parse(buffer);
    free(buffer);
    fclose(fp);

    if (!cJSON_HasObjectItem(*json, "config")) {
        generate_config(*json);
    }
    if (debug) info("load finished");

    cJSON_Delete(config_json);
    return 0;
}

void *save_and_generate_image(void *json) {
    cJSON *config = get_config(json);
    int sleep = cJSON_GetObjectItemCaseSensitive(config, "save_interval")->valueint;
    while (1) {
        lock_json();
        filter_positions(json);
        char *json_str = cJSON_Print(json);
        save(json_str);
        cJSON_free(json_str);
        generate_image(json);
        unlock_json();
        usleep(sleep * 60 * 1000 * 1000);
    }
}