#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cmp.h"

void print_params(Params *params)
{
  fprintf(stderr, "params:\n");
  while (params != NULL)
  {
    fprintf(stderr, "%s: %s\n", params->name, params->tp->kind == TY_INT ? "int" : "ptr");
    params = params->next;
  }
}

size_t to_size(Type *tp)
{
  if (tp == NULL)
    error("to_size: tp is NULL");
  switch (tp->kind)
  {
  case TY_INT:
    return 4;
  case TY_PTR:
    return 8;
  }
  error("to_size: unknown type");
  return 0;
}

Def *applyDefs(Defs *defs, char *name)
{
  while (defs != NULL)
  {
    if (strcmp(defs->def->name, name) == 0)
      return defs->def;
    defs = defs->next;
  }
  return NULL;
}

bool tpcmp(Type *lhs, Type *rhs)
{
  if (lhs == NULL || rhs == NULL)
    return false;
  if (lhs->kind != rhs->kind)
    return false;
  if (lhs->kind == TY_PTR)
    return tpcmp(lhs->inner, rhs->inner);
  return true;
}

void infer_dfns(Defs *dfns)
{
  Defs *cur = dfns;
  while (cur != NULL)
  {
    infer_dfn(cur->def, dfns);
    cur = cur->next;
  }
}
Def *current_def = NULL;
void infer_dfn(Def *dfn, Defs *defs)
{
  if (dfn == NULL)
    return;
  current_def = dfn;
  infer(dfn->body, defs, dfn->locals);
}

// exprの型を検査し、型を返す
// 副作用として、exprとその子孫に型をセットする
Type *infer(Node *node, Defs *defs, Locals *locals)
{
  if (node == NULL)
    return NULL;
  switch (node->kind)
  {
  case ND_NUM:
    node->tp = extendType(NULL, TY_INT);
    return node->tp;
  case ND_VAR:
  {
    Locals *l = applyLocals(locals, node->name);
    if (l == NULL)
      error("infer: undefined variable %s", node->name);
    node->tp = l->tp;
    return node->tp;
  }
  case ND_REF:
  {
    Type *tp = infer(node->lhs, defs, locals);
    if (tp == NULL)
      error("infer: lhs is NULL");
    node->tp = extendType(tp, TY_PTR);
    return node->tp;
  }
  case ND_DEREF:
  {
    Type *tp = infer(node->lhs, defs, locals);
    if (tp == NULL)
      error("infer: lhs is NULL");
    if (tp->kind != TY_PTR)
      error("infer: lhs is not TY_PTR");
    node->tp = tp->inner;
    if (node->tp == NULL)
      error("internal infer: node->tp is NULL");
    return node->tp;
  }
  case ND_CALL:
  {
    Def *def = applyDefs(defs, node->name);
    if (def == NULL)
      error("infer: undefined function %s", node->name);
    Params *params = def->params;
    NodeList *args = node->nds;

    while (params != NULL && args != NULL)
    {
      Type *arg_tp = infer(args->node, defs, locals);
      if (!tpcmp(arg_tp, params->tp))
        error("infer: type mismatch");
      params = params->next;
      args = args->next;
    }
    if (params != NULL || args != NULL)
      error("infer: number of arguments does not match");
    node->tp = def->tp;
    return node->tp;
  }
  case ND_ASS:
  case ND_EQ:
  case ND_NEQ:
  {
    Type *l = infer(node->lhs, defs, locals);
    Type *r = infer(node->rhs, defs, locals);
    if (!tpcmp(l, r))
      error("型エラー");
    node->tp = r;
    return node->tp;
  }
  case ND_ADD:
  case ND_SUB:
  {
    Type *lhs = infer(node->lhs, defs, locals);
    Type *rhs = infer(node->rhs, defs, locals);
    if (lhs == NULL || rhs == NULL)
      error("infer: lhs or rhs is NULL");

    int stat = (lhs->kind == TY_INT) << 1 | (rhs->kind == TY_INT);
    switch (stat)
    {
    case 0b00:
      error("pointers cannot be added/subtracted");
    case 0b11:
      node->tp = extendType(NULL, TY_INT);
      break;
    case 0b01:
      node->tp = lhs;
      break;
    case 0b10:
      node->tp = rhs;
      break;
    }
    return node->tp;
  }

  case ND_LT:
  case ND_GT:
  case ND_LTE:
  case ND_GTE:
  case ND_MUL:
  case ND_DIV:
  {
    Type *lhs = infer(node->lhs, defs, locals);
    Type *rhs = infer(node->rhs, defs, locals);
    if (lhs == NULL || rhs == NULL)
      error("infer: lhs or rhs is NULL");
    if (lhs->kind != TY_INT || rhs->kind != TY_INT)
      error("infer: lhs or rhs is not TY_INT");
    node->tp = extendType(NULL, TY_INT);
    return node->tp;
  }
  case ND_SEQ:
  {
    infer(node->lhs, defs, locals);
    infer(node->rhs, defs, locals);
    return NULL;
  }
  case ND_RET:
  {
    Type *ret_tp = infer(node->lhs, defs, locals);
    if (!tpcmp(current_def->tp, ret_tp))
      error("infer: return する式の型が関数の返り値型にマッチしない");
    return NULL;
  }
  case ND_IF:
  {
    infer(node->lhs, defs, locals);
    infer(node->rhs, defs, locals);
    Type *cond_tp = infer(node->cond, defs, locals);
    if (cond_tp->kind != TY_INT)
      error("infer: cond is not TY_INT");
    return NULL;
  }
  case ND_WHILE:
  {
    infer(node->lhs, defs, locals);
    Type *cond_tp = infer(node->cond, defs, locals);
    if (cond_tp->kind != TY_INT)
      error("infer: cond is not TY_INT");
    return NULL;
  }
  case ND_FOR:
  {
    infer(node->lhs, defs, locals);
    infer(node->rhs, defs, locals);
    infer(node->init, defs, locals);
    Type *cond_tp = infer(node->cond, defs, locals);
    if (!(node->cond == NULL && cond_tp == NULL) && cond_tp->kind != TY_INT)
      error("infer: cond is not TY_INT");
    return NULL;
  }
  case ND_BLK:
  {
    infer(node->lhs, defs, locals);
    return NULL;
  }
  case ND_SIZEOF:
  {
    infer(node->lhs, defs, locals);
    node->tp = extendType(NULL, TY_INT);
    return node->tp;
  }
  }
  error("実装忘れ");
  return NULL; // never
}
