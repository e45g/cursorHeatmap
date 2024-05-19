#include "../include/utils.h"

#include <windows.h>

pthread_mutex_t json_mutex = PTHREAD_MUTEX_INITIALIZER;

void lock_json() {
    pthread_mutex_lock(&json_mutex);
}

void unlock_json() {
    pthread_mutex_unlock(&json_mutex);
}

long get_file_size(FILE *fp) {

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

void screen_size(int *x, int *y) {
    *x = GetSystemMetrics(SM_CXSCREEN);
    *y = GetSystemMetrics(SM_CYSCREEN);
}