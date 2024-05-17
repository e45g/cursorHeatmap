/*
TODO:
0) Refactor code to make it readible :) 60%
1) Reset data option
2) Open on startup option
3) Start with arguments <nogui/...>
4) Auto Background
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>

#include "lib/cJSON.c"
#include "lib/libattopng.c"

#define error(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[*] " msg "\n", ##__VA_ARGS__)
#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)

#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define MAX_RGBA_VALUE 255
#define MAX_KEY_LENGTH 20
#define PNG_FILENAME "heatmap.png"
#define DATA_FILENAME "data.json"

pthread_mutex_t json_mutex = PTHREAD_MUTEX_INITIALIZER;

void lock_json() {
    pthread_mutex_lock(&json_mutex);
}

void unlock_json() {
    pthread_mutex_unlock(&json_mutex);
}

cJSON *get_config(cJSON *json) {
    return cJSON_GetObjectItem(json, "config");
}

int get_value(cJSON *json, char *key) {
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);
    return value ? value->valueint : 0;
}

void process_position(cJSON *json, char *key) {
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (value != NULL && cJSON_IsNumber(value)) {
        cJSON_SetNumberValue(value, cJSON_GetNumberValue(value) + 1);
    } else {
        cJSON_AddNumberToObject(json, key, 1);
    }
}

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

long get_file_size(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    return file_size;
}

int create_data_file(const char *filename, cJSON *config_json) {
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

void screen_size(int *x, int *y) {
    *x = GetSystemMetrics(SM_CXSCREEN);
    *y = GetSystemMetrics(SM_CYSCREEN);
}

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
        if(current_value/highest_value < 0.05) continue;
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

    memcpy(bg, cJSON_GetArrayItem(cJSON_GetObjectItem(config_json, "rgb_background"), 0), sizeof(bg));
    memcpy(fg, cJSON_GetArrayItem(cJSON_GetObjectItem(config_json, "rgb_activity"), 0), sizeof(fg));

    fill_png(png, screen_x, screen_y, bg, MAX_RGBA_VALUE);
    set_pixels_from_json(png, json, fg, highest_value);

    libattopng_save(png, PNG_FILENAME);
    libattopng_destroy(png);

    if (debug) info("Image generation ended");
    return 0;
}

void *save_and_generate_image(void *json) {
    cJSON *config = get_config(json);
    int sleep = cJSON_GetObjectItemCaseSensitive(config, "save_interval")->valueint;
    while (1) {
        lock_json();
        char *json_str = cJSON_Print(json);
        save(json_str);
        free(json_str);
        generate_image(json);
        unlock_json();
        Sleep(sleep * 60 * 1000);
    }
}

void *position_logic(void *json) {
    POINT *coords = malloc(sizeof(POINT));
    int last_coords[2] = {0, 0};
    cJSON *config = get_config(json);
    int sleep_time = get_value(config, "polling_rate");

    while (1) {
        usleep(sleep_time);
        GetCursorPos(coords);

        char pos[20];
        sprintf(pos, "%ldx%ld", coords->x, coords->y);

        if (last_coords[0] != coords->x || last_coords[1] != coords->y) {
            lock_json();
            process_position(json, pos);
            unlock_json();
        }

        last_coords[0] = coords->x;
        last_coords[1] = coords->y;
    }
    free(coords);
}

int main() {
    cJSON *json = cJSON_CreateObject();
    cJSON *config = cJSON_CreateObject();
    pthread_t threads[2];
    char cmds[2][20] = {"Save", "Generate image"};
    
    int load_result = load(DATA_FILENAME, &json);
    if (load_result != 0) {
        error("exiting - %d", load_result);
        exit(1);
    }
 
    int hidden = get_value(config, "hidden");
    if (hidden) {
        HWND hWnd = GetConsoleWindow();
        ShowWindow(hWnd, SW_HIDE);
    }  

    if (pthread_create(&threads[0], NULL, position_logic, json) != 0 || pthread_create(&threads[1], NULL, save_and_generate_image, json) != 0) {
        error("Error while trying to create a thread.");
        exit(1);
    }

    while (1) {
        for (int i = 0; i < (sizeof(cmds) / sizeof(cmds[0])); i++) {
            printf("[%d] %s\n", i, cmds[i]);
        }

        int u_input;
        printf("Command -> ");
        scanf("%d", &u_input);

        system("cls");

        lock_json();
        switch (u_input) {
            case 0:
                char *json_str = cJSON_Print(json);
                if (save(json_str) == 0) {
                    okay("Positions saved.\n");
                }
                free(json_str);
                break;

            case 1:
                if (generate_image(json) == 0) {
                    okay("Image generated.\n");
                }
                break;

            default:
                info("Choose number between %d and %d\n", 0, sizeof(cmds) / sizeof(cmds[0]));
                break;
        }
        unlock_json();
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    cJSON_Delete(json);
    return 0;
}