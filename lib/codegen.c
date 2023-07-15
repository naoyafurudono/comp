#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
  Locals *l = applyLocals(current_locals, varname);
  if (l == NULL)
  {
    error("gen: undefined variable %s", varname);
  }
  return (-1) * (16 + l->offset * 8);
}
size_t label_count = 0;
char *new_label_name()
{
  int buf_size = 10;
  if (label_count == 10000000)
  // バッファサイズに達したら
  // 三文字は.Lと終端文字
  // buf_size-3桁作れる
  {
    error("gen: too many labels");
  }
  char *buf = calloc(buf_size, sizeof(char));
  sprintf(buf, ".L%lu", label_count++);
  return buf;
}

// nodeの実行でスタックに値が積まれる場合、それをスタックから取り除く
void gc_stack(Node *node)
{
  if (node == NULL)
  {
    return;
  }
  if (node->expr)
  {
    pop(0);
  }
}

void prologue(size_t locals)
{
  // SPは16の倍数になっている必要がある
  size_t range = (((locals * 8 - 1) / 16) + 1) * 16;
  push(29);
  printf("    mov x29, SP\n");
  printf("    sub SP, SP, #%lu\n", range); // (全てのローカル変数はXnレジスタに格納されるので。)
}

void epilogue()
{
  printf("    mov SP, x29\n");
  pop(29);
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
// 式を実行する場合、その結果はスタックにpushされる
// 式以外を実行した場合、その内側で式を実行するとしても、最終的にはスタックのサイズを変えてはならない
void gen(Node *node)
{
  if (node == NULL)
    return;
  char *end_then, *end_if, *begin_while, *end_while;
  switch (node->kind)
  {
  // exp
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
  // stmt
  case ND_RET:
    gen(node->lhs);
    pop(0);
    epilogue();
    printf("    ret\n");
    return;
  case ND_SEQ:
    gen(node->lhs);
    // gc_stack(node->lhs);
    gen(node->rhs);
    gc_stack(node->rhs);
    return;
  case ND_IF:
    end_then = new_label_name();
    end_if = new_label_name();
    gen(node->cond);
    pop(0);
    printf("    cbz x0, %s\n", end_then);
    gen(node->lhs);
    gc_stack(node->lhs);
    printf("    b %s\n", end_if);
    printf("%s:\n", end_then);
    if (node->rhs)
    {
      gen(node->rhs);
      gc_stack(node->rhs);
    }
    printf("%s:\n", end_if);
    return;
  case ND_WHILE:
    begin_while = new_label_name();
    end_while = new_label_name();
    printf("%s:\n", begin_while);
    gen(node->cond);
    pop(0);
    printf("    cbz x0, %s\n", end_while);
    gen(node->lhs);
    gc_stack(node->lhs);
    printf("    b %s\n", begin_while);
    printf("%s:\n", end_while);
    return;

  default:;
  }

  // 二つの子を持つAST
  // 実行は常に左->右の順にしている
  gen(node->lhs);
  gen(node->rhs);
  switch (node->kind)
  {
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
