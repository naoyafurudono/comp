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
