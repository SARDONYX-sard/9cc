#include "./9cc.h"

//
// 条件分岐によってアセンブリ言語を標準出力する
//

// 第一引数、第二引数、第三引数…と順に続く配列
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// ユニークなアセンブラのラベルを生成するための変数
static int labelseq = 1;
static char *funcname;

/* 与えられたノードのアドレスをスタックに積む関数 */
static void gen_addr(Node *node) {
  if (node->kind == ND_VAR) {
    // アドレス計算 lea命令
    printf("  lea rax, [rbp-%d]\n", node->var->offset);
    printf("  push rax\n");
    return;
  }

  error("not an local value");
}

static void load(void) {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

static void store(void) {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

/* スタックマシンライクな構文木からのアセンブリ出力関数 */
void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %ld\n", node->val);
      return;
    case ND_EXPR_STMT:
      gen(node->lhs);
      printf("  add rsp, 8\n");
      return;
    case ND_VAR:
      gen_addr(node);
      load();
      return;
    case ND_ASSIGN:
      gen_addr(node->lhs);
      gen(node->rhs);
      store();
      return;
    case ND_IF: {
      int seq = labelseq++;
      if (node->els) {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.else.%d\n", seq);
        gen(node->then);
        printf("  jmp .L.end.%d\n", seq);
        printf(".L.else.%d:\n", seq);
        gen(node->els);
        printf(".L.end.%d:\n", seq);
      } else {
        gen(node->cond);
        printf("  pop rax\n");
        printf(
            "  cmp rax, 0\n");  // if条件がfalse(0)の場合、if文から外れる(goto
                                // end)
        printf("  je  .L.end.%d\n", seq);
        gen(node->then);
        printf(".L.end.%d:\n", seq);
      }
      return;
    }
    case ND_WHILE: {
      int seq = labelseq++;
      printf(".L.begin.%d:\n", seq);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .L.end.%d\n", seq);
      gen(node->then);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_FOR: {
      int seq = labelseq++;
      if (node->init) gen(node->init);
      printf(".L.begin.%d:\n", seq);
      if (node->cond) {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.end.%d\n", seq);
      }
      gen(node->then);
      if (node->inc) gen(node->inc);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next) gen(n);
      return;
    case ND_FUNCALL: {
      int nargs = 0;
      for (Node *arg = node->args; arg; arg = arg->next) {
        gen(arg);
        nargs++;
      };

      // 配列のindexは0から始まるので-1する
      for (int i = nargs - 1; i >= 0; i--) {
        printf("  pop %s\n", argreg[i]);
      }

      // 関数を呼び出す前に RSP を 16 バイト境界に揃える必要があります。これは
      // ABI の要求です。 可変長の関数では RAX を 0 に設定します。

      // アセンブララベル名のインデックス
      int seq = labelseq++;
      printf("  mov rax, rsp\n");
      // 15の2進数は`1111`。これを論理積しても値は変化しない。
      printf("  and rax, 15\n");  //

      // ? jnzがcmp命令なしで動作する理由が不明
      printf("  jnz .L.call.%d\n", seq);  // 16バイトの場合
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);
      printf("  jmp .L.end.%d\n", seq);

      printf(".L.call.%d:\n", seq);  // 8バイトの場合、
      printf("  sub rsp, 8\n");      // 8を引いて16の倍数にしてから
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);  // 関数呼び出し。
      printf("  add rsp, 8\n");
      printf(".L.end.%d:\n", seq);
      printf("  push rax\n");
      return;
    }
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      // JMP命令: 無条件に指定した場所に移動する
      printf("  jmp .L.return.%s\n", funcname);
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

void codegen(Function *prog) {
  printf(".intel_syntax noprefix\n");

  for (Function *fn = prog; fn; fn = fn->next) {
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);
    funcname = fn->name;

    // Prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);

    // Emit code
    for (Node *node = fn->node; node; node = node->next) gen(node);

    // Epilogue
    printf(".L.return.%s:\n", funcname);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}
