#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  int n = atoi(argv[1]);

  printf(".globl _main\n");
  printf(".text\n");
  printf(".balign 4\n");
  printf("_main:\n");
  printf("    mov w0, #%d\n", n);
  printf("    ret\n");
  return 0;
}
