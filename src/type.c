#include "9cc.h"

Type *char_type = &(Type){TY_CHAR, 1};
Type *int_type = &(Type){TY_INT, 8};

/* 渡されたType構造体のkindがTY_INTであるか */
bool is_integer(Type *ty) { return ty->kind == TY_CHAR || ty->kind == TY_INT; }

// ポインタの構造体を作成し、返却する関数
Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->size = 8;
  ty->base = base;
  return ty;
}

/* Type分のメモリを確保し、引数lenの数をty->size構造体に入れる */
Type *array_of(Type *base, int len) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->size = base->size * len;
  ty->base = base;
  ty->array_len = len;
  return ty;
}

/* 渡されたノードに型ノードを追加する関数 */
void add_type(Node *node) {
  if (!node || node->ty) return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next) add_type(n);
  for (Node *n = node->args; n; n = n->next) add_type(n);

  switch (node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_PTR_DIFF:
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_FUNCALL:
    case ND_NUM:
      node->ty = int_type;
      return;
    case ND_PTR_ADD:
    case ND_PTR_SUB:
    case ND_ASSIGN:
      node->ty = node->lhs->ty;
      return;
    case ND_VAR:
      node->ty = node->var->ty;
      return;
    case ND_ADDR:
      if (node->lhs->ty->kind == TY_ARRAY)
        node->ty = pointer_to(node->lhs->ty->base);
      else
        node->ty = pointer_to(node->lhs->ty);
      return;
    case ND_DEREF:
      if (!node->lhs->ty->base)
        error_tok(node->tok, "invalid pointer dereference");
      node->ty = node->lhs->ty->base;
      return;
  }
}
