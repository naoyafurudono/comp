#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cmp.h"

#include <stdio.h>

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}
char *getname(Token *token)
{
  char *name = calloc(token->len + 1, sizeof(char));
  strncpy(name, token->str, token->len);
  name[token->len] = '\0';
  return name;
}

Token *token;
Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' ||
        *p == ';')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (*p == '=')
    {
      if (*(p + 1) == '=')
      {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      else
      {
        cur = new_token(TK_RESERVED, cur, p++, 1);
        continue;
      }
    }
    if (*p == '!')
    {
      if (*(p + 1) == '=')
      {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      else
      {
        error_at(p, "予期しない文字です: %c", *(p + 1));
      }
    }
    if (*p == '<')
    {
      // TODO ここでpをincするのは怖いくない？
      if (++p && *p == '=')
      {
        cur = new_token(TK_RESERVED, cur, p - 1, 2);
        p++;
        continue;
      }
      else
      {
        cur = new_token(TK_RESERVED, cur, p - 1, 1);
        continue;
      }
    }
    if (*p == '>')
    {
      if (++p && *p == '=')
      {
        cur = new_token(TK_RESERVED, cur, p - 1, 2);
        p++;
        continue;
      }
      else
      {
        cur = new_token(TK_RESERVED, cur, p - 1, 1);
        continue;
      }
    }

    if ('a' <= *p && *p <= 'z')
    {
      cur = new_token(TK_IDNT, cur, p++, 1);
      continue;
    }
    if (isdigit(*p))
    {
      char *from = p;
      cur = new_token(TK_NUM, cur, p, -1);
      cur->val = strtol(p, &p, 10);
      cur->len = p - from;
      continue;
    }
    error_at(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

bool eat_op(char *op)
{
  if (token->kind != TK_RESERVED ||
      token->len != strlen(op) ||
      memcmp(token->str, op, token->len))
  {
    // fprintf(stderr, "fail to eat_op %s\ntoken: %s\nlen: %d\n", op, token->str, token->len);
    return false;
  }
  token = token->next;
  return true;
}

char *eat_id()
{
  if (token->kind != TK_IDNT)
  {
    return NULL;
  }
  char *name = getname(token);
  token = token->next;
  return name;
}

// effect exit(1), read/write token
void must_eat(char *op)
{
  if (token->kind != TK_RESERVED ||
      token->len != strlen(op) ||
      memcmp(token->str, op, token->len))
  {
    error_at(token->str, "'%s'ではありません", op);
  }
  token = token->next;
}

// effect read/write token
int must_number()
{
  if (token->kind != TK_NUM)
  {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  // fprintf(stderr, "must_number: %d\n", val);
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}
