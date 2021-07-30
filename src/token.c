#include "./9cc.h"

//
// 引数として渡ってきた文字列を単語ごとに分割する
//

char *user_input;
Token *token;  // 現在着目しているトークン

// エラーを報告し、終了する関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告し、終了する関数
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");  // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/*
  現在のトークンが `op` にマッチするか？
  さらに、トークンを1つ読み進める。
  @param op operator(演算子)
 */
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      strncmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

/*
  現在のトークンが識別子の場合、トークンを1つ読み進める
  それ以外はnull
 */
Token *consume_ident(void) {
  if (token->kind != TK_IDENT) return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

//  現在のトークンが `op` であることを確認し、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      strncmp(token->str, op, token->len))
    error_at(token->str, "'%c'で0はありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
long expect_number(void) {
  if (token->kind != TK_NUM) error_at(token->str, "数ではありません");
  long val = token->val;
  token = token->next;
  return val;
}

// 現在解析中の文字列が`;`のトークン型か？
bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成してcurに繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

/*
  pとqが一致するか、qの文字数分の文字を比較する関数
  @param *p トークンとして渡ってきた文字
  @param *q 比較演算子
 */
bool startswith(char *p, char *q) { return strncmp(p, q, strlen(q)) == 0; }

static bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// 引数`c`は半角英字(is_alpha)、数字、アンダーバーかを判定する関数
static bool is_alnum(char c) { return is_alpha(c) || ('0' <= c && c <= '9'); }

/* *pに渡されたトークンが予約語と一致したらそれを返す関数 */
static char *starts_with_reserved(char *p) {
  // Keyword
  static char *kw[] = {"return", "if", "else", "while"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len])) return kw[i];
  }

  // Multi-letter punctuator(2文字以上の区切り文字。比較演算子)
  static char *ops[] = {"==", "!=", "<=", ">="};

  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
    if (startswith(p, ops[i])) return ops[i];

  return NULL;
}

// `user_input` をトークン化して、新しいトークンを返す
Token *tokenize(void) {
  char *p = user_input;
  Token head = {};
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 予約語または比較演算子
    char *kw = starts_with_reserved(p);
    if (kw) {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // Identifier
    if (is_alpha(*p)) {
      char *q = p++;
      while (is_alnum(*p)) p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    // 1文字の区切り文字の場合
    if (ispunct(*p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = q - p;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
