#include <stdio.h>
#include <windows.h>
#include <pthread.h>
#include "lib/cJSON.c"
#include "lib/libattopng.c"
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#define error(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[*] " msg "\n", ##__VA_ARGS__)
#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)

#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))

cJSON *getConfig(cJSON *json){
    return cJSON_GetObjectItem(json, "config");
}

int getValue(cJSON *json, char *key){
    //info("get value called");
    cJSON *val = cJSON_GetObjectItemCaseSensitive(json, key);
    if(val != NULL){
        //info("value %s found", key);
        return val->valueint;
    }

    //info("value %s not found >:(", key);
    return 0;
}

char *processPos(cJSON *json, char *key){
    cJSON *val = cJSON_GetObjectItemCaseSensitive(json, key);
    if(val != NULL){
        cJSON_ReplaceItemInObject(json, key, cJSON_CreateNumber(val->valueint+1));
    }
    else{
        cJSON_AddNumberToObject(json, key, 1);
    }
    char *json_str = cJSON_Print(json);

    return json_str;
}

int save(char *json_str){

    if(getValue(getConfig(cJSON_Parse(json_str)), "debug")) info("saving started");

    if(access("data.json", F_OK) == -1){
        error("Unable to open the file.");
        return 1;
    }

    FILE *fp = fopen("data.json", "w");
    if(fp == NULL){
        error("Unable to open the file.");
        return 1;
    }
    if(fputs(json_str, fp) == EOF){
        error("Error: `save`; Error writing to the file.");
        return 1;
    }
    fclose(fp);

    if(getValue(getConfig(cJSON_Parse(json_str)), "debug")) info("saved ig");
    return 0;
}

int load(char *filename, cJSON **json){
    if(getValue(getConfig(*json), "debug")) info("loading started.");
    FILE *fp = fopen(filename, "r");
    if(access(filename, F_OK) == -1){
        if(getValue(getConfig(*json), "debug")) info("creating data.json");
        fp = fopen(filename, "w");
        if(fp == NULL){
            error("Error: `load`; Error creating %d", filename);
            return 1;
        }
        if(fputs("{\"config\": {\"polling_rate\": 1000, \"debug\": 0, \"hidden\": 0, \"save_interval\": 5, \"rgb_background\": [0,0,0], \"rgb_activity\": [0, 255, 0]}}", fp) == EOF){
            error("Error: `load`; Error writing to %s", filename);
            return 1;
        }
        //if(getValue(getConfig(*json), "debug")) info("default data written to data.json");
        fclose(fp);
    }
    fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    
    char *buffer = (char *)malloc(sizeof(char)*file_size);
    fread(buffer, 1, file_size, fp); 
    fclose(fp);
        
    *json = cJSON_Parse(buffer);
    free(buffer);

    if(!cJSON_HasObjectItem(*json, "config")){
        cJSON *config = cJSON_CreateObject();
        
        cJSON_AddNumberToObject(config, "polling_rate", 1000);
        cJSON_AddNumberToObject(config, "debug", 0);
        cJSON_AddNumberToObject(config, "hidden", 0);
        cJSON_AddNumberToObject(config, "save_interval", 5);
        cJSON_AddItemToObject(config, "rgb_background", cJSON_CreateIntArray((const int[]){0, 0, 0}, 3));
        cJSON_AddItemToObject(config, "rgb_activity", cJSON_CreateIntArray((const int[]){0, 255, 0}, 3));

        cJSON_AddItemToObject(*json, "config", config);
        cJSON_Delete(config);
    }
    if(getValue(getConfig(*json), "debug")) info("load finished");

    return 0;

}

long getMax(cJSON *json){

    if(getValue(getConfig(json), "debug")) info("getMax called.");
    long highest = -1;
    cJSON *current = NULL;

    cJSON_ArrayForEach(current, json){
        if(strcmp(current->string, "config") == 0) continue;
        int value = current->valueint;
        if(value > highest) highest = value;
    }
    if(getValue(getConfig(json), "debug")) info("getMax finished.");
    return highest;
}

int generateImage(cJSON *json){
    if(getValue(getConfig(json), "debug")) info("Image generation started");
    int screen_x = GetSystemMetrics(SM_CXSCREEN);
    int screen_y = GetSystemMetrics(SM_CYSCREEN);

    cJSON *current = NULL;
    cJSON *config = getConfig(json);
    
    int rgb_background[3];
    for(int i=0; i<3; i++){
        rgb_background[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(config, "rgb_background"), i)->valueint;
    }
    
    int rgb_activity[3];
    for(int i=0; i<3; i++){
        rgb_activity[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(config, "rgb_activity"), i)->valueint;
    }
    
    float highest_val = getMax(json);
    libattopng_t *png = libattopng_new(screen_x, screen_y, PNG_RGBA);
    
    for (int i = 0; i < screen_x * screen_y; ++i) {
        png->data[i * 4 + 0] = rgb_background[0]; // Red
        png->data[i * 4 + 1] = rgb_background[1]; // Green
        png->data[i * 4 + 2] = rgb_background[2]; // Blue
        png->data[i * 4 + 3] = 255; // Alpha
    }   
    
    cJSON_ArrayForEach(current, json){
        if(strcmp(current->string, "config") == 0) continue;
        char key[20];
        strcpy(key, current->string);
        float current_val = getValue(json, key);
        char *x = strtok(key, "x");
        char *y = strtok(NULL, "x");
        //libattopng_set_pixel(png, atoi(x), atoi(y), RGBA(rgb_activity[0], rgb_activity[1], rgb_activity[2], (int)(current_val / (highest_val) *255)));
        libattopng_set_pixel(png, atoi(x), atoi(y), RGBA((int)(current_val/highest_val*rgb_activity[0]), (int)(current_val/highest_val*rgb_activity[1]), (int)(current_val / highest_val*rgb_activity[2]), 255));
    }


    libattopng_save(png, "heatmap.png");
    libattopng_destroy(png);
    if(getValue(getConfig(json), "debug")) info("Image generation ended");
    return 0;
}

void *saveAndGenerateImage(void *json){
    cJSON *config = getConfig(json);
    int sleep = cJSON_GetObjectItemCaseSensitive(config, "save_interval")->valueint;
    while(1){
        if(getValue(config, "debug")) info("new save and img gen started");
        char *json_str = cJSON_Print( (struct cJSON*)json ); 
        save(json_str);
        generateImage(json);
        free(json_str);
        if(getValue(config, "debug")) info("Image generation stopped");
        Sleep(sleep * 60 * 1000);
    }
}

void *positionLogic(void *json){
    POINT *coords = malloc(sizeof(POINT));
    cJSON *config = getConfig(json);
    int last_coords[] = {0, 0};
    
    while(1){
        usleep(getValue(config, "polling_rate"));
        GetCursorPos(coords); 
        
        char pos[20];
        sprintf(pos, "%ldx%ld", coords->x, coords->y);

        if(last_coords[0] != coords->x || last_coords[1] != coords->y){
            char *json_str = processPos(json, pos);
            free(json_str);
        }

        last_coords[0] = coords->x;
        last_coords[1] = coords->y;

    }
    free(coords);   
}

int main(){
    cJSON *json = cJSON_CreateObject();
    cJSON *config = cJSON_CreateObject();
    pthread_t threads[2];


    int val = getValue(config, "hidden");
    if(val){
        HWND hWnd = GetConsoleWindow();
        ShowWindow(hWnd, SW_HIDE);
    }
    
    int load_result = load("data.json", &json);
    if(load_result != 0){
        error("exiting - %d", load_result);
        return 1;
    }    


    if(pthread_create(&threads[0], NULL, positionLogic, json) != 0 || pthread_create(&threads[1], NULL, saveAndGenerateImage, json)){
        error("Error while trying to create a thread.");
        return 1;
    }
    if(getValue(getConfig(json), "debug")) info("threads started");

    char cmds[2][20] = {"Save", "Generate image"};

    while(1){
        for(int i=0; i<(sizeof(cmds)/sizeof(cmds[0])); i++){
            printf("[%d] %s\n", i, cmds[i]);
        }

        int inp;
        printf("Command -> ");
        scanf("%d", &inp);

        printf("\e[1;1H\e[2J - vim ze nefungeje; WIP\n");

        switch(inp){
            case 0:
                if(save(cJSON_Print(json)) == 0){
                    okay("Positions saved.\n");
                }
                break;
                
            case 1:
                if(generateImage(json) == 0){
                    okay("Image generated.\n");
                }
                break;

            default:
                info("Choose number between %d and %d", 0, sizeof(cmds)/sizeof(cmds[0]));
        }
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    cJSON_Delete(json); 
    return 0;
}