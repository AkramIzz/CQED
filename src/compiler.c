#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include "object.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
   // next token to consume
   // use to report errors when parsing tokens
   Token current;
   // last token consumed
   // use to report errors when emitting bytecode
   Token previous;
   bool had_error;
   bool panic_mode;
} Parser;

typedef enum {
   PREC_NONE, PREC_ASSIGNMENT, PREC_OR, PREC_AND,
   PREC_EQUALITY, PREC_COMPARISON, PREC_TERM, PREC_FACTOR,
   PREC_UNARY, PREC_CALL, PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();
typedef struct {
   ParseFn prefix;
   ParseFn infix;
   Precedence precedence;
} ParseRule;

// forward declarations
static void declaration();
static void statement();
static void print_statement();
static void expression_statement();
static void expression();
static void parse_precedence(Precedence precedence);
static void grouping();
static void literal();
static void number();
static void string();
static void unary();
static ParseRule* get_rule(TokenType type);
static void advance();
static bool check(TokenType type);
static bool match(TokenType type);
static void consume(TokenType type, const char *message);
static void end_compiler();
static void emit_byte(uint8_t byte);
static void emit_bytes(uint8_t byte1, uint8_t byte2);
static void emit_constant(Value value);
static uint8_t make_constant(Value value);
static void emit_return();
static void error_at_previous(const char *message);
static void error_at_current(const char *message);
static void error_at(Token *error_token, const char *message);

ParseRule rules[];

Parser parser;

VM *compiling_vm;
static VM *current_vm() {
   return compiling_vm;
}

Chunk *compiling_chunk;
static Chunk *current_chunk() {
   return compiling_chunk;
}

bool compile(const char *source, VM *vm, Chunk *chunk) {
   init_scanner(source);
   compiling_vm = vm;
   compiling_chunk = chunk;

   parser.had_error = false;
   parser.panic_mode = false;
   // initializes parser field current to first token
   advance();
   
   // parse
   while (!match(TOKEN_EOF)) {
      declaration();
   }
   
   end_compiler();
   return !parser.had_error;
}

static void declaration() {
   statement();
}

static void statement() {
   if (match(TOKEN_PRINT)) {
      print_statement();
   } else {
      expression_statement();
   }
}

static void print_statement() {
   expression();
   consume(TOKEN_SEMICOLON, "Expected ';' after print value");
   emit_byte(OP_PRINT);
}

static void expression_statement() {
   expression();
   emit_byte(OP_POP);
   consume(TOKEN_SEMICOLON, "Expected ';' after expression");
}

static void expression() {
   parse_precedence(PREC_ASSIGNMENT);
}

static void parse_precedence(Precedence precedence) {
   advance();
   ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
   if (prefix_rule == NULL) {
   error_at_previous("Expected expression");
   return;
   }

   prefix_rule();

   while (precedence <= get_rule(parser.current.type)->precedence) {
   advance();
   ParseFn infix_rule = get_rule(parser.previous.type)->infix;
   infix_rule();
   }
}

static void binary() {
   TokenType operator_type = parser.previous.type;

   // compile operand
   ParseRule *rule = get_rule(operator_type);
   parse_precedence((Precedence) (rule->precedence + 1));

   switch(operator_type) {
   case TOKEN_EQUAL_EQUAL: emit_byte(OP_EQUAL); break;
   // a != b is equal to !(a == b)
   case TOKEN_BANG_EQUAL: emit_bytes(OP_EQUAL, OP_NOT); break;
   case TOKEN_GREATER: emit_byte(OP_GREATER); break;
   // a >= b is equal to !(a < b)
   case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
   case TOKEN_LESS: emit_byte(OP_LESS); break;
   // a <= b is equal to !(a > b)
   case TOKEN_LESS_EQUAL: emit_bytes(OP_GREATER, OP_NOT); break;
   case TOKEN_PLUS: emit_byte(OP_ADD); break;
   case TOKEN_MINUS: emit_byte(OP_SUBTRACT); break;
   case TOKEN_STAR: emit_byte(OP_MULTIPLY); break;
   case TOKEN_SLASH: emit_byte(OP_DIVIDE); break;
   default:
      // Unreachable
      return;
   }
}

static void grouping() {
   expression();
   consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression");
}

static void literal() {
   switch(parser.previous.type) {
      case TOKEN_FALSE: emit_byte(OP_FALSE); break;
      case TOKEN_NIL: emit_byte(OP_NIL); break;
      case TOKEN_TRUE: emit_byte(OP_TRUE); break;
      default:
         return; // Unreachable
   }
}

static void number() {
   double value = strtod(parser.previous.start, NULL);
   emit_constant(NUMBER_VAL(value));
}

static void string() {
   emit_constant(OBJ_VAL(copy_string(current_vm(), parser.previous.start +1,
      parser.previous.length - 2)));
}

static void unary() {
   TokenType operator_type = parser.previous.type;

   // Compile operand
   parse_precedence(PREC_UNARY);

   switch(operator_type) {
   case TOKEN_BANG: emit_byte(OP_NOT); break;
   case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
   default:
      // Unreachable
      return;
   }
}

static ParseRule* get_rule(TokenType type) {
   return &rules[type];
}

static void advance() {
   parser.previous = parser.current;

   for (;;) {
   parser.current = scan_token();
   if (parser.current.type != TOKEN_ERROR) break;
   // Error messages from scanner are stored in error tokens
   error_at_current(parser.current.start);
   }
}

static bool check(TokenType type) {
   return parser.current.type == type;
}

static bool match(TokenType type) {
   if (check(type)) {
      advance();
      return true;
   }
   return false;
}

static void consume(TokenType type, const char *message) {
   if (check(type)) {
   advance();
   return;
   }

   error_at_current(message);
}

static void end_compiler() {
#ifdef DEBUG_PRINT_CODE
   if (!parser.had_error) {
   disassemble_chunk(current_chunk(), "code");
   }
#endif
   emit_return();
}

static void emit_byte(uint8_t byte) {
   write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
   emit_byte(byte1);
   emit_byte(byte2);
}

static void emit_constant(Value value) {
   emit_bytes(OP_CONSTANT, make_constant(value));
}

static uint8_t make_constant(Value value) {
   int constant_index = add_constant(current_chunk(), value);
   if (constant_index > UINT8_MAX) {
   error_at_previous("Too many constants in one chunk");
   return 0;
   }

   return (uint8_t) constant_index;
}

static void emit_return() {
   emit_byte(OP_RETURN);
}

static void error_at_previous(const char *message) {
   error_at(&parser.previous, message);
}

static void error_at_current(const char *message) {
   error_at(&parser.current, message);
}

static void error_at(Token *error_token, const char *message) {
   if (parser.panic_mode) return;
   parser.panic_mode = true;

   fprintf(stderr, "[line %d] Error", error_token->line);

   if (error_token->type == TOKEN_EOF) {
   fprintf(stderr, " at end");
   } else if (error_token->type == TOKEN_ERROR) {
   // Nothing
   } else {
   fprintf(stderr, " at '%.*s'", error_token->length, error_token->start);
   }

   fprintf(stderr, ": %s\n", message);
   parser.had_error = true;
}

ParseRule rules[] = {
   { grouping, NULL, PREC_CALL }, // TOKEN_LEFT_PAREN
   { NULL,  NULL, PREC_NONE }, // TOKEN_RIGHT_PAREN
   { NULL,  NULL, PREC_NONE }, // TOKEN_LEFT_BRACE
   { NULL,  NULL, PREC_NONE }, // TOKEN_RIGHT_BRACE
   { NULL,  NULL, PREC_NONE }, // TOKEN_COMMA
   { NULL,  NULL, PREC_CALL }, // TOKEN_DOT
   { unary, binary,  PREC_TERM }, // TOKEN_MINUS
   { NULL,  binary,  PREC_TERM }, // TOKEN_PLUS
   { NULL,  NULL, PREC_NONE }, // TOKEN_SEMICOLON
   { NULL,  binary,  PREC_FACTOR }, // TOKEN_SLASH
   { NULL,  binary,  PREC_FACTOR }, // TOKEN_STAR
   { unary,  NULL, PREC_NONE }, // TOKEN_BANG
   { NULL,  binary, PREC_EQUALITY }, // TOKEN_BANG_EQUAL
   { NULL,  NULL, PREC_NONE }, // TOKEN_EQUAL
   { NULL,  binary, PREC_EQUALITY }, // TOKEN_EQUAL_EQUAL
   { NULL,  binary, PREC_COMPARISON }, // TOKEN_GREATER
   { NULL,  binary, PREC_COMPARISON }, // TOKEN_GREATER_EQUAL
   { NULL,  binary, PREC_COMPARISON }, // TOKEN_LESS
   { NULL,  binary, PREC_COMPARISON }, // TOKEN_LESS_EQUAL
   { NULL,  NULL, PREC_NONE }, // TOKEN_IDENTIFIER
   { string,  NULL, PREC_NONE }, // TOKEN_STRING
   { number,NULL, PREC_NONE }, // TOKEN_NUMBER
   { NULL,  NULL, PREC_AND },  // TOKEN_AND
   { NULL,  NULL, PREC_NONE }, // TOKEN_CLASS
   { NULL,  NULL, PREC_NONE }, // TOKEN_ELSE
   { literal,  NULL, PREC_NONE }, // TOKEN_FALSE
   { NULL,  NULL, PREC_NONE }, // TOKEN_FOR
   { NULL,  NULL, PREC_NONE }, // TOKEN_FUN
   { NULL,  NULL, PREC_NONE }, // TOKEN_IF
   { literal,  NULL, PREC_NONE }, // TOKEN_NIL
   { NULL,  NULL, PREC_OR }, // TOKEN_OR
   { NULL,  NULL, PREC_NONE }, // TOKEN_PRINT
   { NULL,  NULL, PREC_NONE }, // TOKEN_RETURN
   { NULL,  NULL, PREC_NONE }, // TOKEN_SUPER
   { NULL,  NULL, PREC_NONE }, // TOKEN_THIS
   { literal,  NULL, PREC_NONE }, // TOKEN_TRUE
   { NULL,  NULL, PREC_NONE }, // TOKEN_VAR
   { NULL,  NULL, PREC_NONE }, // TOKEN_WHILE
   { NULL,  NULL, PREC_NONE }, // TOKEN_ERROR
   { NULL,  NULL, PREC_NONE }, // TOKEN_EOF
};