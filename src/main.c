#include <stdbool.h>
#include <stdio.h>

#include "./struct/node.h"
#include "./struct/token.h"

extern Token *token;
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);

extern void error_at(char *loc, char *fmt, ...);

extern bool consume(char op);
extern void expect(char op);
extern int expect_number();
extern bool at_eof();
extern int expect_number();
extern Token *tokenize(char *p);

int main(int argc, char **argv) {
  if (argc != 2) {
    error_at(token->str, "引数の個数が正しくありません");
    return 1;
  }

  // トークナイズする
  token = tokenize(argv[1]);

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 式の最初は数でなければならないので、それをチェックして
  // 最初のmov命令を出力
  printf("  mov rax, %d\n", expect_number());

  // `+ <数>`あるいは`- <数>`というトークンの並びを消費しつつ
  // アセンブリを出力
  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
