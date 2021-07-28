#!/bin/bash
# @(#) This script is testing framework.
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" >tmp.s
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

# 加減
assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5 "
# 乗除
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 4 "(3+5)/2"
# 単項
assert 10 "-10+20"
assert 10 "- -10"
assert 10 "- - +10"

echo OK
