#ifndef CMP_H
#define CMP_H 1
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef enum
{
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
  TK_IDNT
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
char *getname(Token *token);
Token *tokenize(char *p);

/*

1. ==, !=
2. <, <=, >, >=
3. +, -
4. *, /
5. unary +, unary -
6. ()

program ::= stmt*
stmt ::= expr ";"
expr ::= assign
assign ::= equality ("=" assign)?
equality = relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= ("+" | "-")? primary
primary ::= [a-z] | [0-9]+ | "(" expr ")"
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
  ND_VAR,
  ND_SEQ,
  ND_ASS,
} NodeKind;
extern char *nd_kind_bin_op[];

typedef struct Node Node;
struct Node
{
  NodeKind kind; // must
  int val;       // optional
  char *name;    // optional
  Node *lhs;     // optional
  Node *rhs;     // optional
};

Node *program();

bool eat_op(char *op);
char *eat_id();
void must_eat(char *op);
int must_number();
bool at_eof();

typedef struct Locals Locals;
struct Locals
{
  char *name;
  int offset;
  Locals *next;
};
Locals *getLocal(char *name);
void appendLocals(char *name);

extern Locals *locals;
void prelude();
void postlude();
void gen(Node *node);

#endif