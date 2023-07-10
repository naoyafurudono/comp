#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/cmp.h"

// effect exit(1)
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char *user_input;
// effect exit(1)
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void ptokens(void);

void pprint(Node *);

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);
  Node *node = program();
  if (!at_eof())
  {
    error_at(token->str, "予期しないトークンです: %s", token->str);
  }
  pprint(node);
  return 0;
}

void ptokens()
{
  Token *cur = token;
  while (cur->kind != TK_EOF)
  {
    printf("%s\n", cur->str);
    cur = cur->next;
  }
}
void pprint(Node *node)
{
  if (!node)
    return;
  switch (node->kind)
  {
  case ND_ADD:
    printf("(");
    pprint(node->lhs);
    printf(" + ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_SUB:
    printf("(");
    pprint(node->lhs);
    printf(" - ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_MUL:
    printf("(");
    pprint(node->lhs);
    printf(" * ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_DIV:
    printf("(");
    pprint(node->lhs);
    printf(" / ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_NUM:
    printf("%d", node->val);
    break;
  case ND_EQ:
    printf("(");
    pprint(node->lhs);
    printf(" == ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_NEQ:
    printf("(");
    pprint(node->lhs);
    printf(" != ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_LT:
    printf("(");
    pprint(node->lhs);
    printf(" < ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_LTE:
    printf("(");
    pprint(node->lhs);
    printf(" <= ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_VAR:
    printf("%s", node->name);
    break;
  case ND_SEQ:
    printf("(");
    pprint(node->lhs);
    printf("; ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_ASS:
    printf("(");
    pprint(node->lhs);
    printf(" = ");
    pprint(node->rhs);
    printf(")");
    break;
  case ND_RET:
    printf("return ");
    pprint(node->lhs);
    break;
  case ND_IF:
    printf("(if ");
    pprint(node->cond);
    printf(" ");
    pprint(node->lhs);
    if (node->rhs)
    {
      printf(" ");
      pprint(node->rhs);
    }
    printf(")");
    break;
  case ND_WHILE:
    printf("(while ");
    pprint(node->cond);
    pprint(node->lhs);
    printf(")");
    break;
  case ND_FOR:
    printf("(for (");
    pprint(node->init);
    printf(" ");
    pprint(node->cond);
    printf(" ");
    pprint(node->rhs);
    printf(") ");
    pprint(node->lhs);
    printf(" )");
    break;
  default:
    error("pprint: not defined yet %d", node->kind);
  }
}
