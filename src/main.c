/*
TODO:
-1) filter_positions - own thread? user command only?
0) Refactor code to make it readable :) 80%
1) Reset data option
2) Open on startup option
3) Start with arguments <nogui/...>
4) Auto Wallpaper
*/

#include <pthread.h>    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>

#include "../lib/cJSON.h"
#include "../lib/libattopng.h"

#include "../include/utils.h"
#include "../include/json_ops.h"
#include "../include/file_io.h"
#include "../include/image.h"
#include "../include/cursor_tracking.h"

#define VERSION "0.1.15"


int main() {
    cJSON *json = cJSON_CreateObject();
    pthread_t threads[2];
    char cmds[4][20] = {"Save", "Generate image", "Reset data", "Filter data"};
    
    load(DATA_FILENAME, &json);
    cJSON *config = get_config();
    cJSON_Delete(json);
    json = cJSON_CreateObject();

    int hidden = get_value(config, "hidden");
    if (hidden) {
        HWND hWnd = GetConsoleWindow();
        ShowWindow(hWnd, SW_HIDE);
    }

    SetConsoleTitle("CursorHeatmap: v" VERSION);

    if (pthread_create(&threads[0], NULL, position_logic, json) != 0 || pthread_create(&threads[1], NULL, save_and_generate_image, json) != 0) {
        error("Error while trying to create a thread.");
        exit(1);
    } 

    while (1) {
        for (int i = 0; i < (int)(sizeof(cmds) / sizeof(cmds[0])); i++) {
            printf("[%d] %s\n", i, cmds[i]);
        }

        int u_input;
        printf("Command -> ");
        scanf("%d", &u_input);

        system("cls");

        lock_json();
        switch (u_input) {
            case 0:
                if (save(json) == 0) {
                    okay("Positions saved.\n");
                }
                break;

            case 1:
                if (save(json) == 0 && generate_image() == 0) {
                    okay("Image generated.\n");
                }
                break;
            
            case 2:
                info("WIP\n");
                break;
            
            case 3:
                long removed = filter_positions(json);
                info("Filtered %ld position%s.\n", removed, removed>1 ? "s" : "");
                break;

            default:
                info("Choose a number between %d and %d\n", 0, (int)(sizeof(cmds) / sizeof(cmds[0])));
                break;
        }
        info("Size of json: %d", cJSON_GetArraySize(json));
        unlock_json();
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    cJSON_Delete(json);
    cJSON_Delete(config);
    return 0;
}