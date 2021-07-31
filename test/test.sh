#!/bin/bash
# @(#) This script is testing framework.

# -xc: これ以降に書かれたオプションをcリンカに渡す．
# -c: ソースファイルを、コンパイルまたはアセンブルまではしますが、リンクはしません

# 関数定義をあらかじめ定義し、gccで機械化したものを自作コンパイラで使う
cat <<EOF | gcc -xc -c -o ./build/tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }

int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

assert() {
  expected="$1"
  input="$2"

  ./build/9cc "$input" >./build/tmp.s
  gcc -o ./build/tmp ./build/tmp.s ./build/tmp2.o
  ./build/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# 加減乗除・単項
assert 0 'main () {return 0; }'
assert 42 'main () {return 42; }'
assert 21 'main () {return 5+20-4; }'
assert 41 'main () {return  12 + 34 - 5 ; }'
assert 47 'main () {return 5+6*7; }'
assert 15 'main () {return 5*(9-6); }'
assert 4 'main () {return (3+5)/2; }'
assert 10 'main () {return -10+20; }'
assert 10 'main () {return - -10; }'
assert 10 'main () {return - - +10; }'

# 等価演算子
assert 0 'main () {return 0==1; }'
assert 1 'main () {return 42==42; }'
assert 1 'main () {return 0!=1; }'
assert 0 'main () {return 42!=42; }'

# 比較演算子
assert 1 'main () {return 0<1; }'
assert 0 'main () {return 1<1; }'
assert 0 'main () {return 2<1; }'
assert 1 'main () {return 0<=1; }'
assert 1 'main () {return 1<=1; }'
assert 0 'main () {return 2<=1; }'

assert 1 'main () {return 1>0; }'
assert 0 'main () {return 1>1; }'
assert 0 'main () {return 1>2; }'
assert 1 'main () {return 1>=0; }'
assert 1 'main () {return 1>=1; }'
assert 0 'main () {return 1>=2; }'

assert 1 'main () {return 1; 2; 3; }'
assert 2 'main () {1; return 2; 3; }'
assert 3 'main () {1; 2; return 3; }'

assert 3 'main () {foo=3; return foo; }'
assert 8 'main () {foo123=3; bar=5; return foo123+bar; }'

# if
assert 3 'main () {if (0) return 2; return 3; }'
assert 3 'main () {if (1-1) return 2; return 3; }'
assert 2 'main () {if (1) return 2; return 3; }'
assert 2 'main () {if (2-1) return 2; return 3; }'

# block
assert 3 'main () {{1; {2;} return 3;} }'

# while
assert 10 'main () {i=0; while(i<10) i=i+1; return i; }'
assert 55 'main () {i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

# for
assert 3 'main () {for (;;) return 3; return 5; }'
assert 55 'main () {i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }' # 10の階乗

# function
assert 3 'main () {return ret3(); }'
assert 5 'main () {return ret5(); }'
assert 8 'main () {return add(3, 5); }'
assert 2 'main () {return sub(5, 3); }'
assert 21 'main () {return add6(1,2,3,4,5,6); }'

echo OK
