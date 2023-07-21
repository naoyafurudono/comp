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
  TK_INT
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

program ::= dfn*
dfn ::= type ident "(" (type ident)? ("," type ident)* ")" "{" decl* stmt* "}"
decl ::= type ident";
stmt ::= expr ";"  // expression statement
      | "return" expr ";"  //jump statement
      | "if" "(" expr ")" stmt ("else" stmt)? // selection statement
      | "while" "(" expr ")" stmt  // iteration statement
      | "for" "(" expr? ";" expr? ";" expr? ")" stmt // iteration statement
      | "{" decl* stmt* "}"  // compound statement
expr ::= assign
assign ::= equality ("=" assign)?
equality ::= relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= ("+" | "-")? primary
       | ("*" | "&") unary
primary ::= [a-z]
        | [0-9]+
        | "(" expr ")"
        | ident "(" expr? ("," expr)* ")"
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
  ND_REF,
  ND_DEREF,

  // not expr
  ND_SEQ,
  ND_RET,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLK,
  ND_CALL
} NodeKind;
extern char *nd_kind_bin_op[];
extern char *nd_kind_str[];

typedef struct Defs Defs;
typedef struct Def Def;
typedef struct Params Params;
typedef struct Node Node;
typedef struct NodeList NodeList;
typedef struct Locals Locals;

struct NodeList
{
  Node *node;
  NodeList *next;
};

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
  NodeList *nds;
};

struct Defs
{
  Def *def;
  Defs *next;
};
struct Def
{
  char *name;
  Params *params;
  Node *body;
  Locals *locals;
};

struct Params {
  char *name;
  Params *next;
};

Defs *program();
bool eat_op(char *op);
char *eat_id();
bool eat(TokenKind);
void must_eat(char *op);
int must_number();
bool at_eof();

struct Locals
{
  char *name;
  int offset;
  Locals *next;
};
Locals *applyLocals(Locals *locals, char *name);
Locals *extendLocals(Locals *cur, char *name);
void reset_locals();
extern Locals *current_locals;

void gen_dfn(Def *def);
// void prologue(size_t locals);
// void epilogue();
// void gen(Node *node);

#endif