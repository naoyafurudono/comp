#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
};

Token *token;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool eat(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

void must_eat(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error("'%c'ではありません", op);
  }
  token = token->next;
}

int must_number() {
  if (token->kind != TK_NUM) {
    error("数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
      if (isspace(*p)) {
        p++;
        continue;
      }
      if (*p == '+' || *p == '-') {
        cur = new_token(TK_RESERVED, cur, p++);
        continue;
      }
      if (isdigit(*p)) {
        cur = new_token(TK_NUM, cur, p);
        cur->val = strtol(p, &p, 10);
        continue;
      }
      error("トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  token = tokenize(argv[1]);

  printf(".globl _main\n");
  printf(".text\n");
  printf(".balign 4\n");
  printf("_main:\n");
  printf("    mov w0, #%d\n", must_number());
  while(!at_eof()){

    if (eat('+')) {
      printf("    add w0, w0, #%d\n", must_number());
      continue;
    }

    if (eat('-')) {
      printf("    sub w0, w0, #%d\n", must_number());
      continue;
    }

    fprintf(stderr, "予期しない文字です: '%c'\n", token->str[0]);
    return 1;
  }
  printf("    ret\n");
  return 0;
}
