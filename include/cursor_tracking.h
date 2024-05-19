#ifndef CURSOR_TRACKING_H
#define CURSOR_TRACKING_H

#include "../lib/cJSON.h"

void process_position(cJSON *json, char *key, int addition);
void *position_logic(void *json);

#endif