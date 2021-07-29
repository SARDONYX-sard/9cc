#include "./9cc.h"

//
// 注釈：
// tokenで分割した文字列を、構造体を利用して抽象構文木にする
// gen関数で演算子のアセンブリを生成しているため、ここでは構文木のみを作成
//

// パース結果のノードを順にストアするグローバル配列
Node *code[100];

/* ノードの作成関数 */
static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

/* 二分木ノードの作成関数 */
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = new_node(kind);
  node->lhs = expr;
  return node;
}

/* 整数ノードの作成関数 */
static Node *new_num(long val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

/* 変数ノード作成関数 */
static Node *new_var_node(char name) {
  Node *node = new_node(ND_VAR);
  node->name = name;
  return node;
}

// forward declaration

static Node *stmt(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);

/*
  複数行プログラム全体をパースする関数
  EBNF: program = stmt*
 */
Node *program(void) {
  Node head = {};
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }
  return head.next;
}

/*
  行の区切り文字`;`をパースする関数
  EBNF: stmt = "return" expr ";"
              | expr ";"
 */
static Node *stmt(void) {
  if (consume("return")) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }

  Node *node = new_unary(ND_EXPR_STMT, expr());
  expect(";");
  return node;
}

/*
  assign演算子をパースする関数
  EBNF: expr = equality
 */
static Node *expr(void) { return assign(); }

// assign = equality ("=" assign)?
static Node *assign(void) {
  Node *node = equality();
  if (consume("=")) node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

/*
  比較演算子の`==`と`!=`をパースする関数
  EBNF: equality = relational ("==" relational | "!=" relational)*
 */
static Node *equality(void) {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

/*
  比較演算子の大なり小なりをパースする関数
  EBNF: relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
static Node *relational(void) {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

/*
  加減演算子をパースする関数
  EBNF: add = mul ("+" mul | "-" mul)*
 */
static Node *add(void) {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

/*
  乗除演算子をパースする関数
  EBNF: mul = unary ("*" unary | "/" unary)*
 */
static Node *mul(void) {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

/* 単項演算子をパースする関数
  EBNF: unary   = ("+" | "-")? unary
*/
static Node *unary(void) {
  if (consume("+")) return unary();  // +xをxに置換
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());  // -xを0 - xに置換
  return primary();
}

/*
  算術優先記号`()`と`整数`をパースする関数
  EBNF: primary = "(" expr ")" | ident | num
 */
static Node *primary(void) {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) return new_var_node(*tok->str);

  // そうでなければ数値のはず
  return new_num(expect_number());
}
