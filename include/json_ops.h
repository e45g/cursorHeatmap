#ifndef JSON_OPS_H
#define JSON_OPS_H

#include "../lib/cJSON.h"

long get_highest_value(cJSON *json);
int get_value(cJSON *json, char *key);
long unsigned filter_positions(cJSON *json);
cJSON *get_config();
cJSON *generate_config();

#endif