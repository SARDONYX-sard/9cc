# Makefileのインデントはタブ文字でなければいけません。スペース4個や8個ではエラーになります
CFLAGS=-std=c11 -g -static -o 9cc

9cc: clean
				cc ./src/*.c -o 9cc -g -static

test: 9cc
				bash ./bash/test.sh

fmt:
				bash ./bash/fmt.sh

clean:
				rm -f 9cc *.o *~ tmp*

.PHONY: test clean
