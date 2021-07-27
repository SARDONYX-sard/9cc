#include <stdbool.h>
#include <stdio.h>

#include "./struct/node.h"
#include "./struct/token.h"

// error.c
void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);
// generate.c

extern void gen(Node *node);

// node.c

extern Node *expr();
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
// token.c

extern bool consume(char op);
extern void expect(char op);
extern int expect_number();
extern bool at_eof();
extern int expect_number();
extern Token *token;
extern Token *tokenize();

// 入力プログラム
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) error("%s: 引数の個数が正しくありません", argv[0]);

  // トークナイズと解析を行う
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらアセンブリコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
