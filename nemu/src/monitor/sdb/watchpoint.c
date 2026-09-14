/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"
#include <config/watchpoint.h>

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {
  if (!free_) printf("No free watchpoints\n"), assert(0);
  
  WP *wp = free_;
  free_ = free_->next;
  wp->next = head, head = wp;

  return wp;
}

void free_wp(int wp_NO) {
  if (head == NULL) return;

  WP *cur = head, *prev = NULL;
  while (cur && cur->NO != wp_NO) {
    prev = cur, cur = cur->next;
  }

  if (cur == NULL) return;

  if (prev) prev->next = cur->next;
  else head = cur->next;

  cur->next = free_, free_ = cur;

  memset(cur->expr, 0, sizeof(cur->expr)), cur->val = 0;
}

// Check all the watchpoints
bool check_all_wp() { 
  if (head == NULL) return 0;

  WP *cur = head;
  bool change = false;

  while (cur != NULL) {
    bool success = true; 
    word_t new_val;
    if (cur->val != (new_val = expr(cur->expr, &success)) ) {
      change = 1;
      printf("Watchpoint %d: %s\n", cur->NO, cur->expr);
      printf("Old value: 0x%x\n", cur->val);
      printf("New value: 0x%x\n", new_val);
      cur->val = new_val;
    }
    cur = cur->next;
  }

  return change;
}

void used_wp_display() {
  if (head == NULL) {
    printf("No watchpoints\n");
    return;
  }
  printf("%-10sWhat\n", "Num");
  WP *cur = head;

  while (cur != NULL) {
    printf("%-10d%s\n", cur->NO, cur->expr);
    cur = cur->next;
  }
}

/* TODO: Implement the functionality of watchpoint */

