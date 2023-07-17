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

char *nd_kind_str[] = {
    "ND_EQ",
    "ND_NEQ",
    "ND_LT",
    "ND_GT",
    "ND_LTE",
    "ND_GTE",
    "ND_ADD",
    "ND_SUB",
    "ND_MUL",
    "ND_DIV",
    "ND_NUM",
    "ND_VAR",
    "ND_ASS",
    "ND_SEQ",
    "ND_RET",
    "ND_IF",
    "ND_WHILE",
    "ND_FOR",
    "ND_BLK"};
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
  case ND_BLK:
    node->expr = false;
    break;
  default:
    node->expr = true;
    break;
  }
  return node;
}

// seqにnodeをappendして、末尾のseqを返す
Node *appendSeq(Node *seq, Node *node)
/* invariant:
 *   seq->kind == ND_SEQ &&
 *   seq->rhs == NULL
 * or: // この場合、二要素のリストを作る
 *   seq->kind != ND_SEQ
 */
{
  if (seq == NULL)
    return new_node(ND_SEQ, node, NULL);
  Node *rhs = new_node(ND_SEQ, node, NULL);
  if (seq->kind != ND_SEQ)
  {
    Node *l = new_node(ND_SEQ, seq, new_node(ND_SEQ, rhs, NULL));
    return l->rhs;
  }
  if (seq->lhs == NULL || seq->rhs != NULL)
    error("bad seq provided: %s\n", nd_kind_str[seq->kind]);
  seq->rhs = new_node(ND_SEQ, rhs, NULL);
  return seq->rhs;
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
  Node *p, *node;
  p = node = appendSeq(NULL, stmt());
  while (!at_eof())
    node = appendSeq(node, stmt());
  return p;
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
    Node *body = stmt();
    Node *node = new_node(ND_FOR, body, update);
    node->init = init;
    node->cond = cond;
    return node;
  }
  if (eat_op("{"))
  {
    if (eat_op("}"))
      return new_node(ND_BLK, NULL, NULL);
    Node *child = stmt();
    if (eat_op("}"))
      return new_node(ND_BLK, child, NULL);

    Node *cur, *p;
    cur = p = appendSeq(NULL, child);
    while (!eat_op("}"))
      cur = appendSeq(cur, stmt());
    return new_node(ND_BLK, p, NULL);
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