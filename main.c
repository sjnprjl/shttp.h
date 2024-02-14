#include "shttp.h"
#include "stdio.h"

static char *request_header_msg = "\
POST /api/endpoint HTTP/1.1\r\n\
Host: example.com\r\n\
Content-Type: application/json\r\n\
Authorization: Bearer your_access_token\r\n\
\r\n\
{\
  \"key1\": \"value1\",\
  \"key2\": \"value2\",\
  \"key3\": \"value3\"\
}\
";

int main() {

  http_parser *parser = http_init(&request_header_msg);

  http_request req = {};

  int succes = http_parse_req(parser, &req);

  if (!succes) {
    printf("%s\n", parser->err_reason);
  } else {

    printf("Method: %s, Target: %s, Version: %s\n", req.method, req.uri,
           req.version);

    for (int i = 0; i < req.headers->len; i++) {

      printf("key[%s] => value[%s]\n", req.headers->data[i]->name,
             req.headers->data[i]->value);
    }

    printf("Body: %s\n", req.message_body);
  }

  free_http_req(&req);
  free_http_parser(parser);

  //
  return 0;
}
