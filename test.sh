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
    exit 1
  fi
}

assert 0 "return 0;"
assert 42 "return 42;"
assert 21 "return 5+20-4;"
assert 21 "  return    5  + 20   -    4     ;"
assert 42 "return 3 - 3/321 -3+42;"
assert 126 "return 3 - 3/321 -3+42*3;"
assert 42 "return 3 - 3/321 -3++42;"
assert 42 "return +3 - +3/+321 -+3++42;"
assert 42 "return +3 - +3/+321 -+3++42;"
assert 42 "return 3 * -2 + 96 / 2;"
assert 42 "return (1+2)*(2+12);"
assert 42 "return (1+2)*((2*1)+(3*4));"
assert 0 "return 3  == 2;"
assert 1 "return 3 == 3;"
assert 1 "return 3 != 2;"
assert 0 "return  3 != 3;"
assert 1 "return 3 < 4;"
assert 0 "return 3 < 3;"
assert 0 "return 3 < 2;"
assert 1 "return 3 <= 4;"
assert 1 "return 3 <= 3;"
assert 0 "return 3 <= 2;"
assert 1 "return 3 > 2;"
assert 0 "return 3 > 3;"
assert 0 "return 3 > 4;"
assert 1 "return 3 >= 2;"
assert 1 "return 3 >= 3;"
assert 0 "return 3 >= 4;"
assert 42 "1;1;1; return 42;"
assert 42 "return a = 42;"
assert 42 "return b = 42;"
assert 42 "a = 42;return  a;"
assert 42 "aaaa = 42; return aaaa;"
assert 42 "a = 42; a; a;return a;"
assert 42 "b = 42; return b;"
assert 42 "z = 42; return z;"
assert 20 "a = 20; b = 22;return  a ;"
assert 22 "a = 20; b = 22;  return b;"
assert 42 "a = 20; b = 22; return a + b;"
assert 42 "a = 20; b = 22; return a + b; 3;"

echo OK
