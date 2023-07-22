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
    "ND_BLK",
    "ND_CALL"};
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

  return node;
}

Defs *appendDefs(Defs *defs, Def *def)
{
  Defs *p = calloc(1, sizeof(Defs));
  p->def = def;
  if (defs)
    defs->next = p;
  return p;
}
Defs *program();
Def *dfn();
Params *param(Params *cur);
Type *type();
void decl();
Node *stmt(), *expr(), *assign(), *equality(), *relational(), *add(), *mul(), *unary(), *primary();
Defs *program()
{
  Defs *p, *d;
  p = d = appendDefs(NULL, dfn());
  while (!at_eof())
    d = appendDefs(d, dfn());
  return p;
}

Params *appendParams(Params *params, char *name, Type *tp)
{
  Params *p = calloc(1, sizeof(Params));
  p->name = name;
  p->tp = tp;
  if (params)
    params->next = p;
  return p;
}

Params *param(Params *cur)
{
  Type *tp = type();
  char *name = eat_id();
  if (name == NULL)
    error("仮引数が来るはずでした");
  return appendParams(cur, name, tp);
}

Type *extendType(Type *cur, TypeKind kind)
{
  Type *tp = calloc(1, sizeof(Type));
  switch (kind)
  {
  case TY_INT:
    tp->kind = TY_INT;
    return tp;
  case TY_PTR:
    if (cur == NULL)
      error("extendType: cur is NULL");
    tp->inner = cur;
    tp->kind = TY_PTR;
    return tp;
  default:
    error("extendType: invalid kind");
  }
}
Type *type()
{
  must_eat(TK_INT);
  Type *tp = extendType(NULL, TY_INT);
  while (eat_op("*"))
    tp = extendType(tp, TY_PTR);
  return tp;
}

void decl()
{
  Type *tp = type();
  char *name = eat_id();
  if (name == NULL)
    error("変数名がありません");
  current_locals = extendLocals(current_locals, name, tp);
  must_eat_op(";");
}

Def *dfn()
{
  reset_locals();
  Type *tp = type();
  char *name = eat_id();
  if (name == NULL)
    error("関数名がありません");
  must_eat_op("(");
  Params *params = NULL;
  if (!eat_op(")"))
  {
    Params *cur = params = param(NULL);
    while (eat_op(","))
      cur = param(cur);
    must_eat_op(")");
  }
  Params *cur = params;
  while (cur)
  {
    current_locals = extendLocals(current_locals, cur->name, cur->tp);
    cur = cur->next;
  }
  must_eat_op("{");
  while (tap(TK_INT))
    decl();

  Node *p, *node;
  p = node = appendSeq(NULL, stmt());
  while (!eat_op("}"))
    node = appendSeq(node, stmt());

  Def *def = calloc(1, sizeof(Def));
  def->tp = tp;
  def->name = name;
  def->params = params;
  def->body = p;
  def->locals = current_locals;
  return def;
}

Node *stmt()
{
  if (eat(TK_RETURN))
  {
    Node *node = new_node(ND_RET, expr(), NULL);
    must_eat_op(";");
    return node;
  }
  if (eat(TK_IF))
  {
    must_eat_op("(");
    Node *cond = expr();
    must_eat_op(")");
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
    must_eat_op("(");
    Node *cond = expr();
    must_eat_op(")");
    Node *then_n = stmt();
    Node *node = new_node(ND_WHILE, then_n, NULL);
    node->cond = cond;
    return node;
  }
  if (eat(TK_FOR))
  {
    must_eat_op("(");
    Node *init = NULL;
    Node *cond = NULL;
    Node *update = NULL;
    if (!eat_op(";"))
    {
      init = expr();
      must_eat_op(";");
    }
    if (!eat_op(";"))
    {
      cond = expr();
      must_eat_op(";");
    }
    if (!eat_op(")"))
    {
      update = expr();
      must_eat_op(")");
    }
    Node *body = stmt();
    Node *node = new_node(ND_FOR, body, update);
    node->init = init;
    node->cond = cond;
    return node;
  }
  if (eat_op("{"))
  {
    while (tap(TK_INT))
      decl();
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
  must_eat_op(";");
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
    return node;
}
Node *equality()
{
  Node *node = relational();
  while (true)
    if (eat_op("=="))
      node = new_node(ND_EQ, node, relational());
    else if (eat_op("!="))
      node = new_node(ND_NEQ, node, relational());
    else
      return node;
}
Node *relational()
{
  Node *node = add();
  while (true)
    if (eat_op("<"))
      node = new_node(ND_LT, node, add());
    else if (eat_op("<="))
      node = new_node(ND_LTE, node, add());
    else if (eat_op(">"))
      node = new_node(ND_GT, node, add());
    else if (eat_op(">="))
      node = new_node(ND_GTE, node, add());
    else
      return node;
}
Node *add()
{
  Node *node = mul();
  while (true)
    if (eat_op("+"))
      node = new_node(ND_ADD, node, mul());
    else if (eat_op("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
}
Node *mul()
{
  Node *node = unary();
  while (true)
    if (eat_op("*"))
      node = new_node(ND_MUL, node, unary());
    else if (eat_op("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
}
Node *unary()
{
  if (eat_op("+"))
    return primary();
  if (eat_op("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  if (eat_op("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if (eat_op("&"))
    return new_node(ND_REF, unary(), NULL);
  return primary();
}
Node *primary()
{
  if (eat_op("("))
  {
    Node *node = expr();
    must_eat_op(")");
    return node;
  }
  char *mb_var = eat_id();
  if (mb_var == NULL)
    return new_node_num(must_number());
  if (!eat_op("("))
    return new_node_var(mb_var);
  else
  { // function call
    Node *node = new_node(ND_CALL, NULL, NULL);
    node->name = mb_var;
    if (eat_op(")"))
      return node; // 0 args

    NodeList *args = calloc(1, sizeof(NodeList));
    node->nds = args;
    args->node = expr();
    while (!eat_op(")"))
    { // 2 or more args
      must_eat_op(",");
      args->next = calloc(1, sizeof(NodeList));
      args->next->node = expr();
      args = args->next;
    }
    return node;
  }
}

Locals *current_locals = NULL;
void reset_locals()
{
  current_locals = NULL;
}
/**
 * @brief curにnameをconsする
 *
 * @param cur
 * @param name
 * @return Locals*
 */
Locals *extendLocals(Locals *cur, char *name, Type *tp)
{
  if (applyLocals(cur, name) != NULL)
    return cur;
  Locals *locals = calloc(1, sizeof(Locals));
  locals->name = name;
  locals->tp = tp;
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