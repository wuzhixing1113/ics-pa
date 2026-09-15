#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include "common.h"

typedef struct watchpoint {
    int NO;
    char expr[256];
    word_t val;
    struct watchpoint *next;
} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(int wp_NO);
void used_wp_display();
bool check_all_wp();

#endif