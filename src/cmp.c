#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token
{
  TokenKind kind; // must
  Token *next;    // must
  int val;        // optional
  char *str;      // must
};

Token *token;
Token *new_token(TokenKind kind, Token *cur, char *str)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

typedef enum
{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node
{
  NodeKind kind; // must
  int val;       // optional
  Node *lhs;     // optional
  Node *rhs;     // optional
};

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

bool eat(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
  {
    return false;
  }
  token = token->next;
  return true;
}

// effect exit(1), read/write token
void must_eat(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
  {
    error_at(token->str, "'%c'ではありません", op);
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
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

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
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
    {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

/*
exp ::= term | term ("+" | "-" ) exp
term ::= num | num ("*" | "/") term
num ::= [0-9]+ | "(" exp ")"
*/

typedef enum Kind Kind;
enum Kind
{
  Exp,
  Term,
  Num
};
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}
Node *parse(Kind kind)
{
  Node *lhs, *rhs, *node;
  switch (kind)
  {
  case Exp:
    lhs = parse(Term);
    if (eat('+'))
    {
      rhs = parse(Exp);
      return new_node(ND_ADD, lhs, rhs);
    }
    else if (eat('-'))
    {
      rhs = parse(Exp);
      return new_node(ND_SUB, lhs, rhs);
    }
    else
    {
      return lhs;
    }
  case Term:
    lhs = parse(Num);
    if (eat('*'))
    {
      rhs = parse(Term);
      return new_node(ND_MUL, lhs, rhs);
    }
    else if (eat('/'))
    {
      rhs = parse(Term);
      return new_node(ND_DIV, lhs, rhs);
    }
    else
    {
      return lhs;
    }
  case Num:
    if (eat('('))
    {
      node = parse(Exp);
      must_eat(')');
      return node;
    }
    else
    {
      node = new_node(ND_NUM, NULL, NULL);
      node->val = must_number();
      return node;
    }
  }
}
void pprint(Node *node)
{
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
  }
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);
  Node *node = parse(Exp);
  if (!at_eof()) {
    error_at(token->str, "予期しないトークンです: %s", token->str);
  }
  pprint(node);
  exit(0);

  printf(".globl _main\n");
  printf(".text\n");
  printf(".balign 4\n");
  printf("_main:\n");
  printf("    mov w0, #%d\n", must_number());
  while (!at_eof())
  {
    if (eat('+'))
    {
      printf("    add w0, w0, #%d\n", must_number());
      continue;
    }

    if (eat('-'))
    {
      printf("    sub w0, w0, #%d\n", must_number());
      continue;
    }

    error_at(token->str, "予期しない文字です: %c %n", token->str);
  }
  printf("    ret\n");
  return 0;
}
