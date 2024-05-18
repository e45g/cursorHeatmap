#ifndef FILE_IO_H
#define FILE_IO_H

#include "../lib/cJSON.h"

int save(char *json_str);
void create_data_file(const char *filename, cJSON *config_json);
char *read_file(const char *filename);
int load(const char *filename, cJSON **json);
void *save_and_generate_image(void *json);

#endif