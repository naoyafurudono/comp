#ifndef CMP_H
#define CMP_H
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef enum
{
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token
{
  TokenKind kind; // must
  Token *next;    // must
  int val;        // optional
  char *str;      // must
  int len;        // must
};
extern Token *token;
Token *tokenize(char *p);

/*

1. ==, !=
2. <, <=, >, >=
3. +, -
4. *, /
5. unary +, unary -
6. ()

expr = equality
equality = relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= ("+" | "-")? primary
primary ::= [0-9]+ | "(" expr ")"
*/

typedef enum
{
  ND_EQ,
  ND_NEQ,
  ND_LT,
  ND_GT,
  ND_LTE,
  ND_GTE,
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;
extern char *nd_kind_bin_op[];

typedef struct Node Node;
struct Node
{
  NodeKind kind; // must
  int val;       // optional
  Node *lhs;     // optional
  Node *rhs;     // optional
};

Node *expr();

bool eat(char *op);
void must_eat(char *op);
int must_number();
bool at_eof();

void gen(Node *node);
#endif