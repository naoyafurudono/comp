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
Type *infer(Node *expr, Defs *defs, Locals *locals)
{
  if (expr == NULL)
    return NULL;
  switch (expr->kind)
  {
  case ND_NUM:
    expr->tp = extendType(NULL, TY_INT);
    return expr->tp;
  case ND_VAR:
  {
    Locals *l = applyLocals(locals, expr->name);
    if (l == NULL)
      error("infer: undefined variable %s", expr->name);
    expr->tp = l->tp;
    return expr->tp;
  }
  case ND_REF:
  {
    Type *tp = infer(expr->lhs, defs, locals);
    if (tp == NULL)
      error("infer: lhs is NULL");
    expr->tp = extendType(tp, TY_PTR);
    return expr->tp;
  }
  case ND_DEREF:
  {
    Type *tp = infer(expr->lhs, defs, locals);
    if (tp == NULL)
      error("infer: lhs is NULL");
    if (tp->kind != TY_PTR)
      error("infer: lhs is not TY_PTR");
    expr->tp = tp->inner;
    return expr->tp;
  }
  case ND_CALL:
  {
    Def *def = applyDefs(defs, expr->name);
    if (def == NULL)
      error("infer: undefined function %s", expr->name);
    Params *params = def->params;
    NodeList *args = expr->nds;

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
    expr->tp = def->tp;
    return expr->tp;
  }
  case ND_ASS:
  case ND_EQ:
  case ND_NEQ:
  {
    Type *l = infer(expr->lhs, defs, locals);
    Type *r = infer(expr->rhs, defs, locals);
    if (!tpcmp(l, r))
      error("型エラー");
    expr->tp = r;
    return expr->tp;
  }
  case ND_ADD:
  case ND_SUB:
  {
    Type *lhs = infer(expr->lhs, defs, locals);
    Type *rhs = infer(expr->rhs, defs, locals);
    if (lhs == NULL || rhs == NULL)
      error("infer: lhs or rhs is NULL");

    int stat = (lhs->kind == TY_INT) << 1 | (rhs->kind == TY_INT);
    switch (stat)
    {
    case 0b00:
      error("pointers cannot be added/subtracted");
    case 0b11:
      expr->tp = extendType(NULL, TY_INT);
      break;
    case 0b01:
      expr->tp = lhs;
      break;
    case 0b10:
      expr->tp = rhs;
      break;
    }
    return expr->tp;
  }

  case ND_LT:
  case ND_GT:
  case ND_LTE:
  case ND_GTE:
  case ND_MUL:
  case ND_DIV:
  {
    Type *lhs = infer(expr->lhs, defs, locals);
    Type *rhs = infer(expr->rhs, defs, locals);
    if (lhs == NULL || rhs == NULL)
      error("infer: lhs or rhs is NULL");
    if (lhs->kind != TY_INT || rhs->kind != TY_INT)
      error("infer: lhs or rhs is not TY_INT");
    expr->tp = extendType(NULL, TY_INT);
    return expr->tp;
  }
  case ND_SEQ:
  {
    infer(expr->lhs, defs, locals);
    infer(expr->rhs, defs, locals);
    return NULL;
  }
  case ND_RET:
  {
    Type *ret_tp = infer(expr->lhs, defs, locals);
    if (!tpcmp(current_def->tp, ret_tp))
      error("infer: return する式の型が関数の返り値型にマッチしない");
    return NULL;
  }
  case ND_IF:
  {
    infer(expr->lhs, defs, locals);
    infer(expr->rhs, defs, locals);
    Type *cond_tp = infer(expr->cond, defs, locals);
    if (cond_tp->kind != TY_INT)
      error("infer: cond is not TY_INT");
    return NULL;
  }
  case ND_WHILE:
  {
    infer(expr->lhs, defs, locals);
    infer(expr->rhs, defs, locals);
    infer(expr->init, defs, locals);
    Type *cond_tp = infer(expr->cond, defs, locals);
    if (cond_tp->kind != TY_INT)
      error("infer: cond is not TY_INT");
    return NULL;
  }
  case ND_FOR:
  {
    infer(expr->lhs, defs, locals);
    Type *cond_tp = infer(expr->cond, defs, locals);
    if (!(expr->cond == NULL && cond_tp == NULL) && cond_tp->kind != TY_INT)
      error("infer: cond is not TY_INT");
    return NULL;
  }
  case ND_BLK:
  {
    infer(expr->lhs, defs, locals);
    return NULL;
  }
  default:
    error("実装忘れ");
  }
  return NULL; // never
}
