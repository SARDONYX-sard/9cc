#include "./9cc.h"

// 指定されたファイルの内容を返す
static char *read_file(char *path) {
  // ファイルを開く
  FILE *fp = fopen(path, "r");
  if (!fp) error("cannot open %s: %s", path, strerror(errno));

  int filemax = 10 * 1024 * 1024;
  char *buf = malloc(filemax);
  int size = fread(buf, 1, filemax - 2, fp);
  if (!feof(fp)) error("%s: file too large");

  // ファイルが必ず"\n\0"で終わっているようにする
  if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
  buf[size] = '\0';
  return buf;
}

int align_to(int n, int align) {
  // 10 = 1010
  // alignに8を渡すと-1で７(0111)。ビット反転され8(1000)に。
  // 1000を論理積するので作られる値は、0~8までの数になる。
  return (n + align - 1) & ~(align - 1);
}

int main(int argc, char **argv) {
  if (argc != 2) error("%s: 引数の個数が正しくありません", argv[0]);

  // トークナイズしてパースする
  // 結果はcodeに保存される
  filename = argv[1];
  user_input = read_file(argv[1]);
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

    fn->stack_size = align_to(offset, 8);
  }

  // ASTをトラバースしてアセンブリを出す
  codegen(prog);

  return 0;
}
