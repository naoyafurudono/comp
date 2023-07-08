#include <stdbool.h>
#include <stdio.h>

#include "cmp.h"

void genl(Node *node);
void gen(Node *node)
{
  if (node->kind == ND_NUM)
  {
    printf("    mov x0, #%d\n", node->val);
    printf("    str x0, [SP, #-16]!\n");
    return;
  }
  gen(node->lhs);
  gen(node->rhs);
  switch (node->kind)
  {
  case ND_ADD:
    printf("    ldr x1, [SP], #16\n");
    printf("    ldr x0, [SP], #16\n");
    printf("    add x0, x0, x1\n");
    printf("    str x0, [SP, #-16]!\n");
    break;
  case ND_SUB:
    printf("    ldr x1, [SP], #16\n");
    printf("    ldr x0, [SP], #16\n");
    printf("    sub x0, x0, x1\n");
    printf("    str x0, [SP, #-16]!\n");
    break;
  case ND_MUL:
    printf("    ldr x1, [SP], #16\n");
    printf("    ldr x0, [SP], #16\n");
    printf("    mul x0, x0, x1\n");
    printf("    str x0, [SP, #-16]!\n");
    break;
  case ND_DIV:
    printf("    ldr x1, [SP], #16\n");
    printf("    ldr x0, [SP], #16\n");
    printf("    sdiv x0, x0, x1\n");
    printf("    str x0, [SP, #-16]!\n");
    break;
  case ND_EQ:
  case ND_NEQ:
  case ND_LT:
  case ND_GT:
  case ND_LTE:
  case ND_GTE:
    printf("    ldr x1, [SP], #16\n");
    printf("    ldr x0, [SP], #16\n");
    printf("    cmp x0, x1\n");
    printf("    cset x0, %s\n", nd_kind_bin_op[node->kind]);
    printf("    str x0, [SP, #-16]!\n");
    break;
  default:
    error("parse: invalid node kind");
  }
}

int enc(char *varname)
{
  if (varname[0] < 'a' || 'z' < varname[0])
  {
    error("parse: invalid variable name");
  }
  return (varname[0] - 'a' + 1) * 8;
}
void genl(Node *node)
{
  if (node->kind != ND_VAR)
  {
    error("gen: cannot be a l-value");
  }
  printf("    mov x0, #%d\n", enc(node->name));
  printf("    str x0, [SP, #-16]!\n");
  // printf
}
