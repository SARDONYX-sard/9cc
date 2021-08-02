#define _GNU_SOURCE  // Linux拡張機能へのアクセスのための記述。
                     // strndup関数の使用に必要なため記載。
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED,  // キーワード or 区切り文字(符号など)
  TK_IDENT,     // 識別子(変数や関数など)
  TK_STR,  // 文字列リテラル(数値や文字列を直接に記述した定数のこと)
  TK_NUM,  // 整数トークン
  TK_EOF,  // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind;  // トークンの型
  Token *next;     // 次の入力トークン
  int val;         // kindがTK_NUMの場合、その数値
  char *str;       // トークン文字列
  int len;         // トークンの長さ

  char *contents;  // 終端を含む文字列リテラルの内容 '\0'
  char cont_len;   // string literal length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *peek(char *s);
Token *consume(char *op);
Token *consume_ident(void);
void expect(char *op);
long expect_number(void);
char *expect_ident(void);
bool at_eof(void);
Token *tokenize(void);

extern char *user_input;
extern Token *token;

//
// parse.c
//

// 変数
typedef struct Var Var;
struct Var {
  char *name;     // 変数の名前
  Type *ty;       // Type
  bool is_local;  // local or global

  // ローカル変数
  int offset;  // RBP(ベースレジスタ)からの相対距離(オフセット)

  // Global variable
  char *contents;
  int cont_len;
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;  // 次の変数かNULL
  Var *var;
};

// 抽象構文木のノードの種類　   (注: ptrはポインタの意)
typedef enum {
  ND_ADD,       // num + num
  ND_PTR_ADD,   // ptr + num or num + ptr メモリアドレスの足し算
  ND_SUB,       // num - num
  ND_PTR_SUB,   // ptr - num
  ND_PTR_DIFF,  // ptr - ptr
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_ADDR,  // unary & &を付けられたその変数の格納されているメモリアドレスを取る
  ND_DEREF,      // unary * メモリアドレスから値を取る
  ND_RETURN,     // "return"
  ND_IF,         // "if"
  ND_WHILE,      // "while"
  ND_FOR,        // "for"
  ND_BLOCK,      // "block"
  ND_FUNCALL,    // Function call
  ND_EXPR_STMT,  // Expression statement
  ND_STMT_EXPR,  // Statement expression
  ND_VAR,        // 変数
  ND_NUM,        // Integer
  ND_NULL,       // Empty statement
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind;  // ノードの型
  Node *next;     // 次のノード
  Type *ty;       // Type, e.g. int or pointer to int
  Token *tok;     // Representative token

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
  Function *next;
  char *name;
  VarList *params;

  Node *node;
  VarList *locals;
  int stack_size;
};

typedef struct {
  VarList *globals;
  Function *fns;
} Program;

Program *program(void);

//
// typing.c
//

typedef enum { TY_CHAR, TY_INT, TY_PTR, TY_ARRAY } TypeKind;

struct Type {
  TypeKind kind;
  int size;    // sizeof() value
  Type *base;  // アドレス先の値
  int array_len;
};

extern Type *char_type;
extern Type *int_type;

bool is_integer(Type *ty);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int len);
void add_type(Node *node);

//
// codegen.c
//

void codegen(Program *prog);
