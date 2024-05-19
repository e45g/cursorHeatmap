#ifndef FILE_IO_H
#define FILE_IO_H

#include "../lib/cJSON.h"

int save(cJSON *json);
int create_data_file(const char *filename, cJSON *config_json);
char *read_file(const char *filename);
int load(const char *filename, cJSON **json);

#endif