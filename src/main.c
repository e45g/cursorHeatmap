/*
TODO:
0) Refactor code to make it readible :) 80%
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
#include "../include/config.h"
#include "../include/file_io.h"
#include "../include/image.h"
#include "../include/cursor_tracking.h"

#define VERSION "0.1.13"


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
                info("Choose number between %d and %d\n", 0, (int)(sizeof(cmds) / sizeof(cmds[0])));
                break;
        }
        unlock_json();
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    cJSON_Delete(json);
    return 0;
}