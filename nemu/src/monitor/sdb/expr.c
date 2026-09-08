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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <ctype.h>
#include "memory/paddr.h"

#define MAX_TOKEN_LEN 128
#define MAX_TOKENS 256

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NEQ, TK_NUM, TK_HEX,
  TK_AND, TK_REG, TK_DEREF,
  TK_NEG
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
  {"\\(", '('},         // left paren
  {"\\)", ')'},         // right paren
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"[0-9]+", TK_NUM},   // decimal numbers
  {"0[xX][0-9a-fA-F]+", TK_HEX}, // hexadecimal numbers
  {"&&", TK_AND},       // and
  {"\\$[zero]", TK_REG},       // register
  {"\\*", TK_DEREF},    // dereference
  {"\\-", TK_NEG}         // negative
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[128];
} Token;

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NUM: case TK_HEX: case TK_REG:
            strncpy(tokens[i].str, substr_start, substr_len);
          case '+': case '-': case '*': case '/':
          case '(': case ')': 
          case TK_EQ: case TK_NEQ: 
          case TK_AND: case TK_DEREF:
            tokens[nr_token].type = rules[i].token_type;
          case TK_NOTYPE:
          default: break;
        }
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool binary_op(int op) {
  return op == '+' || op == '-' || op == '*'
  || op == '/' || op == TK_EQ || op == TK_NEQ 
  || op == TK_AND;
}

static bool unary_op(int op) {
  return op == TK_DEREF || op == TK_NEG;
}

static bool check_paren(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;
  int cnt = 0;
  for (int i = p + 1; i < q; i ++) {
    if (tokens[i].type == '(') cnt++;
    else if (tokens[i].type == ')') cnt--;
    if (cnt < 0) return false;
  }
  return true;
}

static int main_op(int p, int q) {
  if (unary_op(tokens[p].type)) return tokens[p].type;
  int i, cnt;
  for (i = q; i >= p; i --) {
    cnt = 0;
    if (tokens[i].type == ')') cnt++;
    else if (tokens[i].type == '(') cnt--;

    if (cnt == 0 && tokens[i].type == TK_AND) return i; 
  }
  for (i = q; i >= p; i --) {
    cnt = 0;
    if (tokens[i].type == ')') cnt++;
    else if (tokens[i].type == '(') cnt--;

    if (cnt == 0 && (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ)) 
      return i; 
  }
  for (i = q; i >= p; i --) {
    cnt = 0;
    if (tokens[i].type == ')') cnt++;
    else if (tokens[i].type == '(') cnt--;

    if (cnt == 0 && (tokens[i].type == '+' || tokens[i].type == '-')) 
      return i; 
  }
  for (i = q; i >= p; i --) {
    cnt = 0;
    if (tokens[i].type == ')') cnt++;
    else if (tokens[i].type =='(') cnt--;

    if (cnt == 0 && (tokens[i].type == '*' || tokens[i].type == '/')) 
      return i; 
  }
  return -1;
}

static word_t calc(int p, int q, bool *success, bool* zero) {
  if (p > q) {
    *success = false;
    return 0;
  }
  else if (p == q) {
    Token cur_tok = tokens[p];
    word_t res = 0;
    switch (cur_tok.type) {
      case TK_NUM:
        for (int i = 0; i < strlen(cur_tok.str); i ++) {
          res = res * 10 + (cur_tok.str[i] - '0');
        }
        return res;
      case TK_HEX:
        for (int i = 0; i < strlen(cur_tok.str); i ++) {
          if (i == 0 || i == 1) continue; // ignore "0x"
          if (isdigit(cur_tok.str[i])) 
            res = res * 16 + (cur_tok.str[i] - '0');
          else { // a-f & A-F
            char ch = tolower(cur_tok.str[i]);
            res = res * 16 + (ch - 'a' + 10);
          }
        }
        return res;
      case TK_REG:
        res = isa_reg_str2val(cur_tok.str, success);
        if (*success) return res;
        return 0;
      default:
        *success = false;
        return 0;
    }
  }
  else if (check_paren(p, q) == true) {
    return calc(p + 1, q - 1, success, zero);
  }
  else {
    int op = main_op(p, q);
    if (op == -1) {
      *success = false;
      return 0;
    }
    word_t val1 = 0, val2 = 0;
    if (binary_op(op)) val1 = calc(p, op - 1, success, zero);
    val2 = calc(p + 1, q, success, zero);
    if (*success && !*zero) {
      switch (op) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/':
          if (val2 == 0) {
            *zero = 1;
            return 0;
          }
          return val1 / val2;
        case TK_AND: return val1 && val2;
        case TK_EQ: return val1 == val2;
        case TK_NEQ: return val1 != val2;
        case TK_NEG: return (word_t)(-val2);
        case TK_DEREF: return paddr_read(val2, 4);
        default:
          *success = false;
          return 0;
      }
    }
    return 0;
  }
}

static bool __attribute__((used)) is_minus(int id) {
  int prev_type = tokens[id - 1].type;
  return prev_type == ')' || prev_type == TK_HEX ||
  prev_type == TK_NUM || prev_type == TK_REG;
}

static bool is_mul(int id) {
  int prev_type = tokens[id - 1].type;
  return prev_type == ')' || prev_type == TK_HEX ||
  prev_type == TK_NUM || prev_type == TK_REG;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    printf("Invalid expression\n");
    return 0;
  }

  for (size_t i = 0; i < nr_token; i ++) {
    bool flag = (i == 0 || !is_mul(i));
    if (tokens[i].type == '*' && flag)
      tokens[i].type = TK_DEREF;
    
    flag = (i == 0 || !is_minus(i));
    if (tokens[i].type == '-' && flag)
      tokens[i].type = TK_NEG;
  }
  bool zero = 0;
  word_t res = calc(0, nr_token - 1, success, &zero);

  if (*success && !zero) return res;
  else if (*success && zero) printf("Division by zero\n"), *success = false;
  else printf("Invalid expression\n");
  return 0;
}
