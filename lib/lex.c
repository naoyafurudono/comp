#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cmp.h"

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

bool is_alpha(char c)
{
  return (('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z') ||
          ('_' == c));
}

bool is_digit(char c)
{
  return ('0' <= c && c <= '9');
}

bool is_id_member(char c)
{
  return (is_alpha(c) || is_digit(c) || ('_' == c));
}

bool is_keyword(char *p, char *keyword)
{
  return (strncmp(p, keyword, strlen(keyword)) == 0 &&
          !is_id_member(*(p + strlen(keyword))));
}

Token *token;
Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    if (is_keyword(p, "return"))
    {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }
    if (is_keyword(p, "if"))
    {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }
    if (is_keyword(p, "else"))
    {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }
    if (is_keyword(p, "while"))
    {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }
    if (is_keyword(p, "for"))
    {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }
    if (is_keyword(p, "int"))
    {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }
    if (is_keyword(p, "sizeof"))
    {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '{' || *p == '}' ||
        *p == ';' || *p == ',' ||
        *p == '&')
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

    if (is_alpha(*p))
    {
      char *from = p;
      for (p++; *p && is_id_member(*p); p++)
        ;
      cur = new_token(TK_IDNT, cur, from, p - from);
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

void must_eat_op(char *op)
{
  if (!eat_op(op))
    error_at(token->str, "'%s'ではありません", op);
}

bool eat_op(char *op)
{
  bool res = tap_op(op);
  if (res)
    token = token->next;
  return res;
}
bool tap_op(char *op)
{
  if (token->kind != TK_RESERVED ||
      token->len != strlen(op) ||
      memcmp(token->str, op, token->len))
  {
    return false;
  }
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

void must_eat(TokenKind kind)
{
  if (!eat(kind))
    error_at(token->str, "トークンが思ったのと違います");
}

bool eat(TokenKind kind)
{
  bool res = tap(kind);
  if (res)
    token = token->next;
  return res;
}

bool tap(TokenKind kind)
{
  return token->kind == kind;
}

// effect exit(1), read/write token
int must_number()
{
  if (token->kind != TK_NUM)
  {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}
