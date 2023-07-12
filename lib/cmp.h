#ifndef CMP_H
#define CMP_H 1
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef enum
{
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
  TK_IDNT,
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
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
stmt ::= expr ";"  // expression statement
      | "return" expr ";"  //jump statement
      | "if" "(" expr ")" stmt ("else" stmt)? // selection statement
      | "while" "(" expr ")" stmt  // iteration statement
      | "for" "(" expr? ";" expr? ";" expr? ")" stmt // iteration statement
      | "{" stmt* "}"  // compound statement
expr ::= assign
assign ::= equality ("=" assign)?
equality ::= relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= ("+" | "-")? primary
primary ::= [a-z]
        | [0-9]+
        | "(" expr ")"
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
  ND_ASS,

  // not expr
  ND_SEQ,
  ND_RET,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLK
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
  Node *cond;    // optional
  Node *init;    // optional
  bool expr;     // must
};

Node *program();

bool eat_op(char *op);
char *eat_id();
bool eat(TokenKind);
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
Locals *applyLocals(Locals *locals, char *name);
Locals *extendLocals(Locals *cur, char *name);
extern Locals *current_locals;
void prelude(size_t locals);
void postlude();
void gen(Node *node);

#endif