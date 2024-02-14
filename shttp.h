#ifndef SHTTP_H
#define SHTTP_H

#include "string.h"
#ifndef TRUE
#define TRUE 1
#endif // !TRUE

#ifndef FALSE
#define FALSE 0

#endif // !FALSE

#define CONCAT(a, b) (a##b)
#define CR '\r'
#define LF '\n'
#define SP ' '
#define HT 9

#define OWS(ch) ((ch == SP) || (ch == HT)) // ; optional whitespace

#define IS_UPALPHA(c) (c >= 'A' && c <= 'Z')
#define IS_LOALPHA(c) (c >= 'a' && c <= 'z')
#define IS_ALPHA(c) (IS_UPALPHA(c) || IS_LOALPHA(c))
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#define IS_CTL(c) ((c >= 0 && c <= 31) || c == 127)
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_HEX(c)                                                              \
  (IS_DIGIT(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define IS_VCHAR(c) ((c >= 0x21) && (c <= 0x75))

#define IS_UNRESERVED(c)                                                       \
  (IS_ALPHANUM(c) || c == '-' || c == '.' || c == '_' || c == '~')

#define IS_TCHAR(c)                                                            \
  (c == '!' || c == '#' || c == '$' || c == '%' || c == '&' || c == '\'' ||    \
   c == '*' || c == '+' || c == '^' || c == '`' || c == '|' ||                 \
   IS_UNRESERVED(c))

#define IS_SUB_DELIMS(c)                                                       \
  (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||    \
   c == '*' || c == '+' || c == ',' || c == ';' || c == '=')

#define IS_PCT_ENCODED(ch) ((*ch) == '%' && IS_HEX(*(++ch)) && IS_HEX(*(++ch)))

#define IS_PCHAR(ch)                                                           \
  (IS_UNRESERVED((*ch)) || IS_PCT_ENCODED(ch) || IS_SUB_DELIMS((*ch)))

#include <stdlib.h>

typedef enum http_parse_status {
  h_ok,
  h_err,
} http_parse_status;

typedef struct http_request {
  char *method;
  char *uri;
  char *version;

  //
} http_request;

typedef struct http_parser {
  char **source;
  http_parse_status status;
  const char *err_reason;
} http_parser;

// Def
http_parser *http_init(char **source);
void free_http_parser(http_parser *parser);
void set_err(http_parser *parser, http_parse_status status, const char *err_r);
int http_parse_req(http_parser *, http_request *);
void free_http_req(http_request *req);

// Impl
http_parser *http_init(char **source) {
  http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
  parser->source = source;

  return parser;
};

void free_http_parser(http_parser *parser) {
  free(parser->source);
  free(parser);
}

void set_err(http_parser *parser, http_parse_status status, const char *err_r) {
  parser->status = status;
  parser->err_reason = err_r;
};

static char lex[256];
static int lexi = 0;

void reset_lex() {
  lex[0] = '\0';
  lexi = 0;
}

int http_parse_req(http_parser *parser, http_request *req) {

  // parse method
  while (**parser->source != '\0') {
    if (!IS_TCHAR(**parser->source)) {
      set_err(parser, h_err, "Expected character but got non.");
      return FALSE;
    }
    while (IS_TCHAR(**parser->source)) {
      lex[lexi++] = *(*parser->source)++;
    }
    if (**parser->source != SP) {
      set_err(parser, h_err, "Space character is required after method");
      return FALSE;
    }
    req->method = strdup(lex);
    (*parser->source)++;

    reset_lex();

    // request-target

    if (**parser->source == '*') { // astrik-form
      req->uri = (char *)('*');
    } else if (**parser->source == '/') { // origin-form
      while (**parser->source == '/') {
        lex[lexi++] = *(*parser->source)++;
        while (IS_PCHAR(*parser->source)) {
          lex[lexi++] = *(*parser->source)++;
        }
      }

      req->uri = strdup(lex);
    }
    // todo!: absolute-form

    reset_lex();
    if (**parser->source != SP) {
      set_err(parser, h_err,
              "Space character is required after request-target");
      return FALSE;
    }
    (*parser->source)++; // next

    if (strncmp(*parser->source, "HTTP", 4) != 0) {
      set_err(parser, h_err, "HTTP version should start with 'HTTP' token");
      return FALSE;
    }
    lex[lexi++] = 'H';
    lex[lexi++] = 'T';
    lex[lexi++] = 'T';
    lex[lexi++] = 'P';
    (*parser->source) += 4;

    if (**parser->source != '/') {
      set_err(parser, h_err, "/ token is expected after HTTP");
      return FALSE;
    }
    lex[lexi++] = *(*parser->source)++; // / next

    if (!IS_DIGIT(**parser->source)) {
      set_err(parser, h_err, "digit expected.");
      return FALSE;
    }
    lex[lexi++] = *(*parser->source)++; // DIGIT

    if (**parser->source != '.') {
      set_err(parser, h_err, "'.' expected.");
      return FALSE;
    }
    lex[lexi++] = *(*parser->source)++; // .

    if (!IS_DIGIT(**parser->source)) {
      set_err(parser, h_err, "Digit expected after '.'");
      return FALSE;
    }
    lex[lexi++] = *(*parser->source)++; // DIGIT

    req->version = strdup(lex);

    reset_lex();

    if (*(*parser->source)++ != CR) {
      set_err(parser, h_err, "CR character expected");
      return FALSE;
    }
    if (*(*parser->source)++ != LF) {
      set_err(parser, h_err, "LF Character expected.");
      return FALSE;
    }

    return TRUE;

    //
  }

  return TRUE;

  //
};

void free_http_req(http_request *req) {
  free(req->method);
  free(req->version);
  free(req->uri);
  //
  free(req);
}
#endif // !SHTTP_H
