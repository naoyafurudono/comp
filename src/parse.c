
#include <stdbool.h>
#include <stdlib.h>
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
  return node;
}
Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}
Node *expr(), *equality(), *relational(), *add(), *mul(), *unary(), *primary();
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