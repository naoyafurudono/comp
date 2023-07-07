#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./cmp "$input" > tmp.s
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

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 21 "   5  + 20   -    4     "
assert 42 "3 - 3/321 -3+42"
assert 126 "3 - 3/321 -3+42*3"
assert 42 "3 - 3/321 -3++42"
assert 42 "+3 - +3/+321 -+3++42"
assert 42 "+3 - +3/+321 -+3++42"
assert 42 "3 * -2 + 96 / 2"
assert 0 "3  == 2"
assert 1 "3 == 3"
assert 1 "3 != 2"
assert 0 "3 != 3"
assert 1 "3 < 4"
assert 0 "3 < 3"
assert 0 "3 < 2"
assert 1 "3 <= 4"
assert 1 "3 <= 3"
assert 0 "3 <= 2"
assert 1 "3 > 2"
assert 0 "3 > 3"
assert 0 "3 > 4"
assert 1 "3 >= 2"
assert 1 "3 >= 3"
assert 0 "3 >= 4"

echo OK
