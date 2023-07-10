#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cmp.h"

char *nd_kind_bin_op[] = {
    "eq",
    "ne",
    "lt",
    "gt",
    "le",
    "ge"};
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  switch (kind)
  {
  case ND_SEQ:
  case ND_RET:
  case ND_IF:
  case ND_WHILE:
  case ND_FOR:
    node->expr = false;
    break;
  default:
    node->expr = true;
    break;
  }
  return node;
}
Node *new_node_num(int val)
{
  Node *node = new_node(ND_NUM, NULL, NULL);
  node->val = val;
  return node;
}
Node *new_node_var(char *name)
{
  Node *node = new_node(ND_VAR, NULL, NULL);
  node->name = name;

  current_locals = extendLocals(current_locals, name);
  return node;
}

Node *program(), *stmt(), *expr(), *assign(), *equality(), *relational(), *add(), *mul(), *unary(), *primary();
Node *program()
{
  Node *node = stmt();
  if (at_eof())
  {
    return node;
  }
  return new_node(ND_SEQ, node, program());
}

Node *stmt()
{
  if (eat(TK_RETURN))
  {
    Node *node = new_node(ND_RET, expr(), NULL);
    must_eat(";");
    return node;
  }
  if (eat(TK_IF))
  {
    must_eat("(");
    Node *cond = expr();
    must_eat(")");
    Node *then_n = stmt();
    Node *node = new_node(ND_IF, then_n, NULL);
    node->cond = cond;
    if (eat(TK_ELSE))
    {
      Node *else_n = stmt();
      node->rhs = else_n;
    }
    return node;
  }
  if (eat(TK_WHILE))
  {
    must_eat("(");
    Node *cond = expr();
    must_eat(")");
    Node *then_n = stmt();
    Node *node = new_node(ND_WHILE, then_n, NULL);
    node->cond = cond;
    return node;
  }
  if (eat(TK_FOR))
  {
    must_eat("(");
    Node *init = NULL;
    Node *cond = NULL;
    Node *update = NULL;
    if (!eat_op(";"))
    {
      init = expr();
      must_eat(";");
    }
    if (!eat_op(";"))
    {
      cond = expr();
      must_eat(";");
    }
    if (!eat_op(")"))
    {
      update = expr();
      must_eat(")");
    }
    Node *then_n = stmt();
    Node *node = new_node(ND_FOR, then_n, NULL);
    node->init = init;
    node->cond = cond;
    node->rhs = update;
    return node;
  }
  Node *node = expr();
  must_eat(";");
  return node;
}
Node *expr()
{
  return assign();
}
Node *assign()
{
  Node *node = equality();
  if (eat_op("="))
  {
    Node *rhs = assign();
    return new_node(ND_ASS, node, rhs);
  }
  else
  {
    return node;
  }
}
Node *equality()
{
  Node *node = relational();
  while (true)
  {
    if (eat_op("=="))
    {
      node = new_node(ND_EQ, node, relational());
    }
    else if (eat_op("!="))
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
    if (eat_op("<"))
    {
      node = new_node(ND_LT, node, add());
    }
    else if (eat_op("<="))
    {
      node = new_node(ND_LTE, node, add());
    }
    else if (eat_op(">"))
    {
      node = new_node(ND_GT, node, add());
    }
    else if (eat_op(">="))
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
    if (eat_op("+"))
    {
      node = new_node(ND_ADD, node, mul());
    }
    else if (eat_op("-"))
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
    if (eat_op("*"))
    {
      node = new_node(ND_MUL, node, unary());
    }
    else if (eat_op("/"))
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
  if (eat_op("+"))
  {
    return primary();
  }
  if (eat_op("-"))
  {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}
Node *primary()
{
  if (eat_op("("))
  {
    Node *node = expr();
    must_eat(")");
    return node;
  }
  char *mb_var = eat_id();
  if (mb_var)
  {
    return new_node_var(mb_var);
  }
  return new_node_num(must_number());
}

Locals *current_locals = NULL;
Locals *extendLocals(Locals *cur, char *name)
{
  Locals *locals = calloc(1, sizeof(Locals));
  locals->name = name;
  if (cur)
    locals->offset = cur->offset + 1;
  else
    locals->offset = 0;
  locals->next = cur;
  return locals;
}

Locals *applyLocals(Locals *locals, char *name)
{
  Locals *cur = locals;
  while (cur)
  {
    if (strcmp(cur->name, name) == 0)
      return cur;
    cur = cur->next;
  }
  return NULL;
}