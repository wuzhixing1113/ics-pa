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

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char *expr[128];
  int val;
  /* TODO: Add more members if necessary */

} WP;

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

void free_wp(WP *wp) {
  if (head == NULL) return;

  WP *cur = head, *prev = NULL;
  while (cur != wp) {
    prev = cur, cur = cur->next;
  }

  if (cur == NULL) return;

  if (prev) prev->next = cur->next;
  else head = cur->next;

  cur->next = free_, free_ = cur;

  cur->expr[0] = '\0', cur->val = 0;
}

/* TODO: Implement the functionality of watchpoint */

