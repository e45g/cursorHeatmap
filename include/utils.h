#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdio.h>

#define error(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[*] " msg "\n", ##__VA_ARGS__)
#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)

#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define MAX_RGBA_VALUE 255
#define MAX_KEY_LENGTH 32
#define PNG_FILENAME "heatmap.png"
#define DATA_FILENAME "data.json"

typedef struct {
    int x;
    int y;
    float value;
} PixelData;

extern pthread_mutex_t json_mutex;

void lock_json();
void unlock_json();
long get_file_size(FILE *fp);
void screen_size(int *x, int *y);

#endif