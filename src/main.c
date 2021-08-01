#include "./9cc.h"

// 入力プログラム
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) error("%s: 引数の個数が正しくありません", argv[0]);

  // トークナイズしてパースする
  // 結果はcodeに保存される
  user_input = argv[1];
  token = tokenize();
  Program *prog = program();

  // ローカル変数の個数分オフセット(メモリ領域)を割り当てる
  for (Function *fn = prog->fns; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next) {
      Var *var = vl->var;
      offset += var->ty->size;
      var->offset = offset;
    }
    fn->stack_size = offset;
  }

  // ASTをトラバースしてアセンブリを出す
  codegen(prog);

  return 0;
}
