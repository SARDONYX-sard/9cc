#include "./9cc.h"

// 入力プログラム
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) error("%s: 引数の個数が正しくありません", argv[0]);

  // トークナイズしてパースする
  // 結果はcodeに保存される
  user_input = argv[1];
  token = tokenize();
  Node *node = program();

  // Traverse the AST to emit assembly.
  codegen(node);
  return 0;
}
