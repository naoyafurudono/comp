#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./cmp "$input" >tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    NOK=1
  fi
}

# assert 55 "a=0; b=1; for(i=0; i < 10; i = i+1){ t = b; b = a+b; a= t;} return b;"
# for ((i=-10; i < 10; i++)); do
#   assert $(( (i + 256) % 256 )) "a=b=$i; return b;"
# done
# assert 10 "for(i=0;i<10;i=i+1){i=i+1;} return i;"

assert 42 "int f(int x){ return x; } int main(){ return f(42); }"
assert 42 "int f(int a, int b){ return *(&a - 8);} int main(){ return f(0, 42); }"
assert 42 "int main(){ x = 42; return *(&x); }"
assert 55 "int fib(int n){ if (n < 2) return n; a = fib(n-1); b = fib(n-2); return a+b; } int main(){ return fib(10); }"
assert 120 "int fact(int n){ if (n <= 1) return 1; return n * fact(n-1); } int main(){ return fact(5); }"
assert 120 "int fact(int n){ if (n <= 1) return n; return n * fact(n-1); } int main(){ return fact(5); }"
assert 120 "int fact(int n){ res = 1; while(n>0){ res = res * n; n = n - 1; } return res; } int main(){ return fact(5); }"
assert 42 "int echo(int n){ return n; } int main(){ return echo(42); }"
assert 42 "int ft(){ return 42; } int main(){ return ft(); }"
assert 55 "int main(){ a=0; b=1; i=0; while(i < 10){ t=a+b; a=b; b=t; i=i+1; } return a; }"
assert 45 "int main(){ a=0; b=0; i=0; while(i < 10){ b = b+i;i=i+1; } return b; }"
assert 1 "int main(){ return 1; return 2; return 3; }"
assert 2 "int main(){ 1; return 2; return 3; }"
assert 2 "int main(){ {1; return 2; return 3;} }"
assert 0 "int main(){ for(;;){} return 0; }"
assert 0 "int main(){ for(1;0;3){1;} return 0; }"
assert 0 "int main(){ return 0; }"
assert 42 "int main(){ return 42; }"
assert 21 "int main(){ return 5+20-4; }"
assert 21 "int main(){   return    5  + 20   -    4     ; }"
assert 42 "int main(){ return 3 - 3/321 -3+42; }"
assert 42 "int main(){ {return 3 - 3/321 -3+42;} }"
assert 126 "int main(){ return 3 - 3/321 -3+42*3; }"
assert 42 "int main(){ return 3 - 3/321 -3++42; }"
assert 42 "int main(){ return +3 - +3/+321 -+3++42; }"
assert 42 "int main(){ return +3 - +3/+321 -+3++42; }"
assert 42 "int main(){ return 3 * -2 + 96 / 2; }"
assert 42 "int main(){ return (1+2)*(2+12); }"
assert 42 "int main(){ return (1+2)*((2*1)+(3*4)); }"
assert 1 "int main(){ return 3 == 3; }"
assert 1 "int main(){ return 3 != 2; }"
assert 1 "int main(){ return 3 < 4; }"
assert 1 "int main(){ return 3 <= 4; }"
assert 1 "int main(){ return 3 <= 3; }"
assert 1 "int main(){ return 1 <= 1; }"
assert 1 "int main(){ return 3 > 2; }"
assert 1 "int main(){ return 3 >= 2; }"
assert 1 "int main(){ return 3 >= 3; }"
assert 0 "int main(){ return 3  == 2; }"
assert 0 "int main(){ return  3 != 3; }"
assert 0 "int main(){ return 3 < 3; }"
assert 0 "int main(){ return 3 < 2; }"
assert 0 "int main(){ return 3 <= 2; }"
assert 0 "int main(){ return 3 > 3; }"
assert 0 "int main(){ return 3 > 4; }"
assert 0 "int main(){ return 3 >= 4; }"
assert 42 "int main(){ 1;1;1; return 42; }"
assert 42 "int main(){ 1;1;{1; return 42;} }"
assert 42 "int main(){ return a = 42; }"
assert 42 "int main(){ return b = 42; }"
assert 42 "int main(){ a = 42;return  a; }"
assert 42 "int main(){ aaaa = 42; return aaaa; }"
assert 42 "int main(){ a = 42; a; a;return a; }"
assert 42 "int main(){ b = 42; return b; }"
assert 42 "int main(){ z = 42; return z; }"
assert 42 "int main(){ if (1) return 42; }"
assert 42 "int main(){ if (1) return 42; else return 0; }"
assert 42 "int main(){ if (1 == 0) a = 24; else a = 42; return a; }"
assert 10 "int main(){ a = 0; while (a < 10) a = a + 1; return a; }"
assert 55 "int main(){ a=0;b=0; while((a = (a+1)) < 11) b = b + a; return b; }"
assert 20 "int main(){ a=9;z=0; while((a = (a+1)) < 11)  if (a!=7) z = a + a; return z; }"
assert 12 "int main(){ a=0;z=0; while((a = (a+1)) < 7)  if (a!=7) z = a + a; return z; }"
assert 12 "int main(){ a=0;z=0; while((a = (a+1)) < 8)  if (a!=7) z = a + a; return z; }"
assert 14 "int main(){ a=0;z=0; while((a = (a+1)) < 8)  if (a!=8) z = a + a; return z; }"
assert 42 "int main(){ a=0;b=0; while((a = (a+1)) < 11) if((a != 7) + (a != 6) != 1) b = b + a; return b; }"
assert 20 "int main(){ a = 20; b = 22;return  a ; }"
assert 22 "int main(){ a = 20; b = 22;  return b; }"
assert 42 "int main(){ a = 20; b = 22; return a + b; }"
assert 42 "int main(){ a = 20; b = 22; return a + b; 3; }"
assert 3 "int main(){ a=0; a=a+1; a=a+a+1; return a; }"

if [[ $NOK ]]; then
  exit 1
fi
echo OK
