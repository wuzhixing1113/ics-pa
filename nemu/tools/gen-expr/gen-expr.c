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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static void gen_num() {
  int choice = rand() % 4;
  if (choice <= 1) {
    uint32_t num = rand() % 1000;
    sprintf(buf + strlen(buf), "%u", num);
  }else if (choice == 2) {
    uint32_t num = rand() % 256;
    sprintf(buf + strlen(buf), "0x%x", num);
  }else {
    int cnt = rand() % 3;
    for (int i = 0; i < cnt; i++) 
      strcat(buf, "-");
    uint32_t num = rand() % 1000;
    sprintf(buf + strlen(buf), "%u", num);
  }
  choice = rand() % 4;
  for (int i = 0; i < choice; i++) 
    strcat(buf, " ");
}

static void gen_op() {
  char *ops[] = {"+", "-", "*", "/", "==", "!=", "&&"};
  int idx = rand() % 8;
  strcat(buf, ops[idx]);
}

static void gen_rand_expr(int d) {
  if (d >= 5) {
    gen_num();
    return;
  }
  int choice = rand() % 3;
  switch (choice) {
  case 0:
    gen_num();
    break;
  case 1:
    strcat(buf, "("); gen_rand_expr(d + 1); strcat(buf, ")");
    break;
  default:
    gen_rand_expr(d + 1); gen_op(); gen_rand_expr(d + 1);
    break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
