#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
   const char *start;
   const char *current;
   int line;
} Scanner;

Scanner scanner;

void init_scanner(const char *source) {
   scanner.start = source;
   scanner.current = source;
   scanner.line = 1;
}

Token scan_token() {
   Token token = (Token) {
      .type = TOKEN_EOF,
      .start = NULL,
      .length = 0,
      .line = 1
   };
   return token;
}