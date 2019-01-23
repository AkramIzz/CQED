#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "chunk.h"

static void advance();
static void consume(TokenType type, const char *message);
static void end_compiler();
static void emit_byte(uint8_t byte);
static void emit_bytes(uint8_t byte1, uint8_t byte2);
static void emit_return();
static void error_at_previous(const char *message);
static void error_at_current(const char *message);
static void error_at(Token *error_token, const char *message);

typedef struct {
   // next token to consume
   Token current;
   // last token consumed
   Token previous;
   bool had_error;
   bool panic_mode;
} Parser;

Parser parser;

Chunk *compiling_chunk;
static Chunk *current_chunk() {
   return compiling_chunk;
}

bool compile(const char *source, Chunk *chunk) {
   init_scanner(source);
   compiling_chunk = chunk;

   parser.had_error = false;
   parser.panic_mode = false;
   // initializes parser field current to first token
   advance();
   
   // parse
   // expression();
   consume(TOKEN_EOF, "Expected end of expression");
   
   end_compiler();
   return !parser.had_error;
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

static void consume(TokenType type, const char *message) {
   if (parser.current.type == type) {
      advance();
      return;
   }

   error_at_current(message);
}

static void end_compiler() {
   emit_return();
}

static void emit_byte(uint8_t byte) {
   write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
   emit_byte(byte1);
   emit_byte(byte2);
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