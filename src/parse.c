#include "./9cc.h"

//
// 注釈：
// tokenで分割した文字列を、構造体を利用して抽象構文木にする
// gen関数で演算子のアセンブリを生成しているため、ここでは構文木のみを作成
//

// 解析中に作成されたすべてのローカル変数インスタンスは
// この配列に蓄積されます。
Var *locals;

// Find a local variable by name.
static Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (strlen(var->name) == tok->len &&
        !strncmp(tok->str, var->name, tok->len))
      return var;
  return NULL;
}

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
static Node *new_var_node(Var *var) {
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

/* ローカル変数のノード作成関数 */
static Var *new_lvar(char *name) {
  Var *var = calloc(1, sizeof(Var));  // Varの構造体一つずつに1byteメモリを確保
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
}

// forward declaration

static Function *function(void);
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
  EBNF: program = function*
 */
Function *program(void) {
  Function head = {};
  Function *cur = &head;

  while (!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }
  return head.next;
}

// function = ident "(" ")" "{" stmt* "}"
static Function *function(void) {
  locals = NULL;

  char *name = expect_ident();
  expect("(");
  expect(")");
  expect("{");

  Node head = {};
  Node *cur = &head;

  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }

  Function *fn = calloc(1, sizeof(Function));
  fn->name = name;
  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

static Node *read_expr_stmt(void) { return new_unary(ND_EXPR_STMT, expr()); }

/*
  予約語と、行の区切り文字`;`をパースする関数
  EBNF: stmt = "return" expr ";"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "{" stmt* "}"
              | expr ";"
 */
static Node *stmt(void) {
  if (consume("return")) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }

  if (consume("if")) {
    Node *node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else")) node->els = stmt();
    return node;
  }

  if (consume("while")) {
    Node *node = new_node(ND_WHILE);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (consume("for")) {
    Node *node = new_node(ND_FOR);
    expect("(");

    // "for"初期値構文の始めに";"が来ていないかを確かめることで、
    // EBNFの"?"というオプショナルを実現している
    if (!consume(";")) {  // "for"の初期値
      node->init = read_expr_stmt();
      expect(";");
    }
    if (!consume(";")) {  // "for"の条件部分
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {  // "for"の累積量部分
      node->inc = read_expr_stmt();
      expect(")");
    }
    node->then = stmt();
    return node;
  }

  if (consume("{")) {
    Node head = {};
    Node *cur = &head;

    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }

    Node *node = new_node(ND_BLOCK);
    node->body = head.next;
    return node;
  }

  Node *node = read_expr_stmt();
  expect(";");
  return node;
}

/*
  assign演算子をパースする関数
  EBNF: expr = assign
 */
static Node *expr(void) { return assign(); }

/*
  `=`演算子をパースする関数
  EBNF: assign = equality ("=" assign)?
 */
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

/*
  単項演算子をパースする関数
  EBNF: unary   = ("+" | "-")? unary
*/
static Node *unary(void) {
  if (consume("+")) return unary();  // +xをxに置換
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());  // -xを0 - xに置換
  return primary();
}

/*
  引数の有無に応じて処理が分岐し、パースする関数
  EBNF: func-args = "(" (assign ("," assign)*)? ")"
*/
static Node *func_args(void) {
  if (consume(")")) return NULL;

  Node *head = assign();
  Node *cur = head;
  while (consume(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
}

/*
  算術優先記号`()`と`関数`、`変数`、`整数`をパースする関数
  EBNF: primary = "(" expr ")" | ident args?  | num
 */
static Node *primary(void) {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    // 識別子の次に"()"がきたら Function
    if (consume("(")) {
      Node *node = new_node(ND_FUNCALL);
      // strndupは第2引数のサイズ指定分、文字列を複製する
      node->funcname = strndup(tok->str, tok->len);
      node->args = func_args();  // 引数ノードの作成は`func_args`に任せる
      return node;
    }

    Var *var = find_var(tok);
    if (!var)
      var = new_lvar(
          strndup(tok->str, tok->len));  // トーク文字列名を変数名にする
    return new_var_node(var);
  }

  // そうでなければ数値のはず
  return new_num(expect_number());
}
