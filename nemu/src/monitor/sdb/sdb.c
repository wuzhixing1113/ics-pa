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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <monitor/sdb/watchpoint.h>
#include "sdb.h"
#include "memory/paddr.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
WP* new_wp();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args);

static int cmd_help(char *args);

static int cmd_info(char *args);

static int cmd_p(char *args);

static int cmd_x(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single-step N instructions, then pause. Default N = 1", cmd_si},
  { "info", "Show the information of registers or watchpoint", cmd_info},
  { "p", "Calculate the value of the expression EXPR", cmd_p},
  { "w", "Stop execution when the value of EXPR changes", cmd_w},
  { "x", "Evaluate EXPR as address, print N 4-byte words in hex", cmd_x},
  { "d", "Delete the corresponding watchpoint", cmd_d}
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  if (args == NULL) { // Default: N = 1
    cpu_exec(1);
  }
  else {
    uint64_t N;
    for (size_t i = 0; i < strlen(args); i ++) {
      if (args[i] == ' ' || args[i] == '\0') break;
      if (!isdigit(args[i])) {
        printf("Syntax error near %c\n", args[i ? i - 1 : 0]);
        return 0;
      }
    }

    sscanf(args, "%lu", &N);

    cpu_exec(N);
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");

  if(arg == NULL) {
    printf("Invalid usage of info\n");
    return 0;
  }

  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  }else if (strcmp(arg, "w") == 0) {
    used_wp_display();
  }else {
    printf("Undefined info command \"%s\"\n", arg);
  }
  return 0;
}

static int cmd_p(char *args) {
  bool success = true;
  uint32_t res = expr(args, &success);
  
  if(success) printf("%u\n", res);

  return 0;
}

static int cmd_w(char *args) {
  bool success = true;
  word_t val = expr(args, &success);
  if(!success) return 0;

  WP *NEW_wp = new_wp();
  strcpy(NEW_wp->expr, args);
  NEW_wp->val = val;
  printf("Watchpoint %d: %s\n", NEW_wp->NO, NEW_wp->expr);
  return 0;
}

static int cmd_x(char *args) {
  char *arg1 = strtok(NULL, " "), *arg2 = strtok(NULL, " ");
  if (arg1 == NULL || arg2 == NULL) {
    printf("Undefined x command\n");
    return 0;
  } 
  else {
    int len;
    paddr_t addr;
    for (size_t i = 0; i < strlen(arg1); i ++) {
      if (arg1[i] == ' ' || arg1[i] == '\0') break;
      if (!isdigit(arg1[i])) {
        printf("Syntax error near %c\n", arg1[i ? i - 1 : 0]);
        return 0;
      }
    }
    sscanf(arg1, "%d", &len);
    bool success = true;
    addr = expr(arg2, &success);
    
    if (success) {
      for (int i = 0; i < len; i ++) 
        printf("0x%x: 0x%08x\n", addr + 4 * i, paddr_read(addr + 4 * i, 4));
    }
  }
  return 0;
}

static int cmd_d(char *args) {
  int wp_NO, i;
  for (i = 0; i < strlen(args); i ++) {
    if (args[i] == ' ' || args[i] == '\0') break;
    if (!isdigit(args[i])) {
      printf("Invalid usage\n");
      return 0;
    }
  }
  sscanf(args, "%d", &wp_NO);
  free_wp(wp_NO);
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
