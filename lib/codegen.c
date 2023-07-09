#include <stdbool.h>
#include <stdio.h>

#include "cmp.h"

void pop(int reg)
{
  printf("    ldr x%d, [SP], #16\n", reg);
}
void push(int reg)
{
  printf("    str x%d, [SP, #-16]!\n", reg);
}
int enc(char *varname)
{
  if (varname[0] < 'a' || 'z' < varname[0])
  {
    error("parse: invalid variable name");
  }
  return (-1) * (16 + (varname[0] - 'a') * 8);
}
void prelude()
{
  push(29);
  printf("    mov x29, SP\n");
  printf("    sub SP, SP, #208\n");
}
void postlude()
{
  push(0);
  // printf("    add SP, SP, #112\n");
  printf("    mov SP, x29\n");
  pop(29);
  printf("    ret\n");
}
void genl(Node *node)
// lvalueの評価結果はbase pointerからのoffset
{
  if (node->kind != ND_VAR)
  {
    error("gen: cannot be a l-value");
  }
  printf("    mov x0, #%d\n", enc(node->name));
  push(0);
}
// x29をベースポインタを保持するレジスタとして使う
void gen(Node *node)
{
  switch (node->kind)
  {
  case ND_NUM:
    printf("    mov x0, #%d\n", node->val);
    push(0);
    return;
  case ND_ASS:
    genl(node->lhs);
    gen(node->rhs);
    pop(0);
    pop(1);
    printf("    str x0, [x29, x1]\n");
    push(0);
    return;
  case ND_VAR:
    printf("    ldur x0, [x29, %d]\n", enc(node->name));
    push(0);
    return;
  default:;
  }

  // 二つの子を持つAST
  // 実行は常に左->右の順にしている
  gen(node->lhs);
  gen(node->rhs);
  switch (node->kind)
  {
  case ND_SEQ:
    return;
  case ND_ADD:
    pop(1);
    pop(0);
    printf("    add x0, x0, x1\n");
    push(0);
    return;
  case ND_SUB:
    pop(1);
    pop(0);
    printf("    sub x0, x0, x1\n");
    push(0);
    return;
  case ND_MUL:
    pop(1);
    pop(0);
    printf("    mul x0, x0, x1\n");
    push(0);
    return;
  case ND_DIV:
    pop(1);
    pop(0);
    printf("    sdiv x0, x0, x1\n");
    push(0);
    return;
  case ND_EQ:
  case ND_NEQ:
  case ND_LT:
  case ND_GT:
  case ND_LTE:
  case ND_GTE:
    pop(1);
    pop(0);
    printf("    cmp x0, x1\n");
    printf("    cset x0, %s\n", nd_kind_bin_op[node->kind]);
    push(0);
    return;
  default:
    error("parse: invalid node kind");
  }
}
