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
  int len;        // must
};

Token *token;
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

typedef enum
{
  ND_EQ,
  ND_NEQ,
  ND_LT,
  ND_GT,
  ND_LTE,
  ND_GTE,
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;
char *nd_kind_bin_op[] = {
    "eq",
    "ne",
    "lt",
    "gt",
    "le",
    "ge"};

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

bool eat(char *op)
{
  if (token->kind != TK_RESERVED ||
      token->len != strlen(op) ||
      memcmp(token->str, op, token->len))
  {
    // fprintf(stderr, "fail to eat %s\ntoken: %s\nlen: %d\n", op, token->str, token->len);
    return false;
  }
  token = token->next;
  return true;
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
    if (*p == '=')
    {
      if (++p && *p == '=')
      {
        cur = new_token(TK_RESERVED, cur, p - 1, 2);
        p++;
        continue;
      }
      else
      {
        error_at(p, "予期しない文字です: %c", *p);
      }
    }
    if (*p == '!')
    {
      if (++p && *p == '=')
      {
        cur = new_token(TK_RESERVED, cur, p - 1, 2);
        p++;
        continue;
      }
      else
      {
        error_at(p, "予期しない文字です: %c", *p);
      }
    }
    if (*p == '<')
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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
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

/*

1. ==, !=
2. <, <=, >, >=
3. +, -
4. *, /
5. unary +, unary -
6. ()

expr = equality
equality = relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= ("+" | "-")? primary
primary ::= [0-9]+ | "(" expr ")"
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
Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}
Node *expr(), *equality(), *relational(), *mul(), *add(), *unary(), *primary();
Node *expr()
{
  return equality();
}
Node *equality()
{
  Node *node = relational();
  while (true)
  {
    if (eat("=="))
    {
      node = new_node(ND_EQ, node, relational());
    }
    else if (eat("!="))
    {
      node = new_node(ND_NEQ, node, relational());
    }
    else
    {
      return node;
    }
  }
}
Node *relational()
{
  Node *node = add();
  while (true)
  {
    if (eat("<"))
    {
      node = new_node(ND_LT, node, add());
    }
    else if (eat("<="))
    {
      node = new_node(ND_LTE, node, add());
    }
    else if (eat(">"))
    {
      node = new_node(ND_GT, node, add());
    }
    else if (eat(">="))
    {
      node = new_node(ND_GTE, node, add());
    }
    else
    {
      return node;
    }
  }
}
Node *add()
{
  Node *node = mul();
  while (true)
  {
    if (eat("+"))
    {
      node = new_node(ND_ADD, node, mul());
    }
    else if (eat("-"))
    {
      node = new_node(ND_SUB, node, mul());
    }
    else
    {
      return node;
    }
  }
}
Node *mul()
{
  Node *node = unary();
  while (true)
  {
    if (eat("*"))
    {
      node = new_node(ND_MUL, node, unary());
    }
    else if (eat("/"))
    {
      node = new_node(ND_DIV, node, unary());
    }
    else
    {
      return node;
    }
  }
}
Node *unary()
{
  if (eat("+"))
  {
    return primary();
  }
  if (eat("-"))
  {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}
Node *primary()
{
  if (eat("("))
  {
    Node *node = expr();
    must_eat(")");
    return node;
  }
  else
  {
    return new_node_num(must_number());
  }
}

void gen(Node *node)
{
  if (node->kind == ND_NUM)
  {
    printf("    mov w0, #%d\n", node->val);
    printf("    str w0, [SP, #-16]!\n");
    return;
  }
  gen(node->lhs);
  gen(node->rhs);
  switch (node->kind)
  {
  case ND_ADD:
    printf("    ldr w1, [SP], #16\n");
    printf("    ldr w0, [SP], #16\n");
    printf("    add w0, w0, w1\n");
    printf("    str w0, [SP, #-16]!\n");
    break;
  case ND_SUB:
    printf("    ldr w1, [SP], #16\n");
    printf("    ldr w0, [SP], #16\n");
    printf("    sub w0, w0, w1\n");
    printf("    str w0, [SP, #-16]!\n");
    break;
  case ND_MUL:
    printf("    ldr w1, [SP], #16\n");
    printf("    ldr w0, [SP], #16\n");
    printf("    mul w0, w0, w1\n");
    printf("    str w0, [SP, #-16]!\n");
    break;
  case ND_DIV:
    printf("    ldr w1, [SP], #16\n");
    printf("    ldr w0, [SP], #16\n");
    printf("    sdiv w0, w0, w1\n");
    printf("    str w0, [SP, #-16]!\n");
    break;
  case ND_EQ:
  case ND_NEQ:
  case ND_LT:
  case ND_GT:
  case ND_LTE:
  case ND_GTE:
    printf("    ldr w1, [SP], #16\n");
    printf("    ldr w0, [SP], #16\n");
    printf("    cmp w0, w1\n");
    printf("    cset w0, %s\n", nd_kind_bin_op[node->kind]);
    printf("    str w0, [SP, #-16]!\n");
    break;
  default:
    error("parse: invalid node kind");
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
  Node *node = expr();
  if (!at_eof())
  {
    error_at(token->str, "予期しないトークンです: %s", token->str);
  }
  printf(".globl _main\n");
  printf(".text\n");
  printf(".balign 4\n");
  printf("_main:\n");
  gen(node);
  printf("    ldr w0, [SP], #16\n");
  printf("    ret\n");
  return 0;
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
