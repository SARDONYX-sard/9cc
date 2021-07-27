# Makefileのインデントはタブ文字でなければいけません。スペース4個や8個ではエラーになります
CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
				./bash/./test.sh

fmt:
				bash ./bash/fmt.sh

clean:
				rm -f 9cc *.o *~ tmp*

.PHONY: test clean
