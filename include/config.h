#ifndef CONFIG_H
#define CONFIG_H

#include "../lib/cJSON.h"

long get_highest_value(cJSON *json);
int get_value(cJSON *json, char *key);
cJSON *get_config(cJSON *json);
cJSON *generate_config(cJSON *json);

#endif