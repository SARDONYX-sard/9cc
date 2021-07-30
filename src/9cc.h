#define _GNU_SOURCE  // Linux拡張機能へのアクセスのための記述。
                     // strndup関数の使用に必要なため記載。
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// utils/strndup.c
//

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED,  // キーワード or 区切り文字(符号など)
  TK_IDENT,     // 識別子(変数や関数など)
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind;  // トークンの型
  Token *next;     // 次の入力トークン
  int val;         // kindがTK_NUMの場合、その数値
  char *str;       // トークン文字列
  int len;         // トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
long expect_number(void);
bool at_eof(void);
Token *tokenize(void);

extern char *user_input;
extern Token *token;

//
// parse.c
//

// ローカル変数
typedef struct Var Var;
struct Var {
  Var *next;   // 次の変数かNULL
  char *name;  // 変数の名前
  int offset;  // RBP(ベースレジスタ)からの相対距離(オフセット)
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_ASSIGN,     // =
  ND_RETURN,     // "return"
  ND_IF,         // "if"
  ND_WHILE,      // "while"
  ND_FOR,        // "for"
  ND_BLOCK,      // "block"
  ND_FUNCALL,    // Function call
  ND_EXPR_STMT,  // Expression statement
  ND_VAR,        // 変数
  ND_NUM,        // Integer
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind;  // ノードの型
  Node *next;     // 次のノード

  Node *lhs;  // 左辺
  Node *rhs;  // 右辺

  // "if" or "while" or "for" statement
  Node *cond;  // condition（条件)
  Node *then;  // 条件がtrueの場合の処理
  Node *els;
  Node *init;  // "for"文の初期値
  Node *inc;   // final-expression(counterなど)

  // Block
  Node *body;

  // Function call
  char *funcname;
  Node *args;

  Var *var;  // ノードの型が変数の場合のみ使う
  long val;  // kindがND_NUMの場合のみ使う
};

typedef struct Function Function;
struct Function {
  Node *node;
  Var *locals;
  int stack_size;
};

Function *program(void);

//
// codegen.c
//

void codegen(Function *prog);
