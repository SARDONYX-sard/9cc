# Makefileのインデントはタブ文字でなければいけません。スペース4個や8個ではエラーになります
CFLAGS=-std=c11 -g -static
# ./src/error.c ./src/main.c ./src/node.c ./src/token.c
9cc:
				cc ./src/error.c ./src/main.c ./src/node.c ./src/token.c -o 9cc

test: 9cc
				bash ./bash/test.sh

fmt:
				bash ./bash/fmt.sh

clean:
				rm -f 9cc *.o *~ tmp*

.PHONY: test clean
