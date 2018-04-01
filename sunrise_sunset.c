#define __USE_XOPEN
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include "jsmn.h"
#include <time.h>
#include "sunrise_sunset.h"

#define SEND_MSG_MAX_BYTES (4096)
#define RECV_MSG_MAX_BYTES (4096)
#define PAGE_MAX_BYTES     (4096)
#define MAX_TOKENS         (4096)

static void
fail (
    int test,
    const char *format,
    ...
)
{
  if (test) {
    va_list args;
    va_start (args, format);
    vfprintf (stderr, format, args);
    va_end (args);
    // exit ok
    exit (EXIT_FAILURE);
  }
}

static void
get_page (
    int s,
    const char *host,
    const char *page,
    char *recv_msg
)
{
  int status;
  const char *format = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: fetch.c\r\n\r\n";

  char *send_msg = malloc(SEND_MSG_MAX_BYTES);
  fail(!send_msg, "malloc for send_msg\n");

  status = snprintf (send_msg, SEND_MSG_MAX_BYTES, format, page, host);
  fail(status == -1 || !send_msg, "snprintf failed\n");

  status = send (s, send_msg, strlen(send_msg), 0);
  fail(status == -1, "send failed: %s\n", strerror (errno));
  free(send_msg);

  int recv_cnt;
  while (1) {
    recv_cnt = recvfrom(s, recv_msg, RECV_MSG_MAX_BYTES - 10, 0, 0, 0);
    if (recv_cnt == 0)
      break;
    fail(recv_cnt == -1, "%s\n", strerror (errno));
    recv_msg[recv_cnt] = '\0';
    //printf ("%s", recv_msg);
  }
}

static int
get_json_in_page(
    char *recv_msg
)
{
  char *json = strstr(recv_msg, "\r\n\r\n{");
  if (json)
    return &json[4] - recv_msg;

  return -1;
}

// /* UNUSED */
// static int
// display_json(
//     const char *js,
//     jsmntok_t *t,
//     size_t count,
//     int indent
// )
// {
//   int tok_cnt;
//   int i;
// 
//   if (!count)
//     return 0;
// 
//   switch (t->type) {
//     case JSMN_PRIMITIVE:
//       printf("%.*s", t->end - t->start, js + t->start);
//       return 1;
//     case JSMN_STRING:
//       printf("'%.*s'", t->end - t->start, js + t->start);
//       return 1;
//     case JSMN_OBJECT:
//       printf("\n");
//       tok_cnt = 0;
//       for (i=0; i<t->size; i++) {
//         int k;
//         for (k=0; k<indent; k++)
//           printf("  ");
//         tok_cnt += display_json(js, &t[tok_cnt + 1], count - tok_cnt, indent + 1);
//         printf(":");
//         tok_cnt += display_json(js, &t[tok_cnt + 1], count - tok_cnt, indent + 1);
//         printf("\n");
//       }
//       return tok_cnt + 1;
//     case JSMN_ARRAY:
//       printf("\n");
//       tok_cnt = 0;
//       for (i=0; i<t->size; i++) {
//         int k;
//         for (k=0; k<indent-1; k++)
//           printf("  ");
//         printf("   - ");
//         tok_cnt += display_json(js, &t[tok_cnt + 1], count - tok_cnt, indent + 1);
//         printf("\n");
//       }
//       return tok_cnt + 1;
//     default:
//       break;
//   }
//   
//   return 0;
// }
// 
// /* UNUSED */
// static int
// display_tokens(
//     const char *json,
//     jsmntok_t *tokens,
//     size_t count
// )
// {
//   int ii;
//   for (ii=0; ii<count; ii++) {
//     char *str_type;
//     switch (tokens[ii].type) {
//     case JSMN_UNDEFINED:
//       str_type = "UNDEFINED";
//       break;
//     case JSMN_OBJECT   :
//       str_type = "OBJECT   ";
//       break;
//     case JSMN_ARRAY    :
//       str_type = "ARRAY    ";
//       break;
//     case JSMN_STRING   :
//       str_type = "STRING   ";
//       break;
//     case JSMN_PRIMITIVE:
//       str_type = "PRIMITIVE";
//       break;
//     }
// 
//     printf("token %d:\n", ii);
//     printf("  type  : %s\n", str_type);
//     printf("  start : %d\n", tokens[ii].start);
//     printf("  end   : %d\n", tokens[ii].end);
//     printf("  size  : %d\n", tokens[ii].size);
// #ifdef JSMN_PARENT_LINKS
//     printf("  parent: %d\n", tokens[ii].parent);
// #endif //JSMN_PARENT_LINKS
//     printf("  %.*s\n", tokens[ii].end - tokens[ii].start, &json[tokens[ii].start]);
//   }
// 
//   return 0;
// }

static int
get_dates(
    sunrise_sunset_t *sun,
    const char *json,
    jsmntok_t *tokens,
    size_t ntok
)
{
  int ii;
  int jj;

  for (ii=0; ii<(ntok-1); ii++) {
    switch (tokens[ii].type) {
    case JSMN_STRING:
      for (jj=0; jj<sizeof(sun->times)/sizeof(sun->times[0]); jj++) {
        if (!strncmp(date_keywords[jj], &json[tokens[ii].start], tokens[ii].end - tokens[ii].start)) {
          // parse ISO 8601 date/time format
          char *stop_char = strptime(&json[tokens[ii+1].start], "%FT%k:%M:%S%z", &sun->times[jj]);
          // UTC to localtime
          time_t time = timegm(&sun->times[jj]);
          localtime_r(&time, &sun->times[jj]);
          //printf("%27s: %s", date_keywords[jj], asctime(&sun->times[jj]));

          int unproc_chars = (int)(&json[tokens[ii+1].end] - stop_char);
          if (unproc_chars) {
            fprintf(stderr, "%d unprocessed characters: '%.*s'\n", unproc_chars, unproc_chars, stop_char);
            return -1;
          }
          break;
        }
      }
      if (jj == sizeof(sun->times)/sizeof(sun->times[0])) {
        if (!strncmp(day_length_keyword, &json[tokens[ii].start], tokens[ii].end - tokens[ii].start)) {
          char *tail = (char *)&json[tokens[ii+1].end];
          sun->day_length = (int)strtol(&json[tokens[ii+1].start], &tail, 10);
          //printf("%27s: %d seconds\n", day_length_keyword, sun->day_length);
        }
      }
    default:
      break;
    }
  }
  
  return 0;
}
  
int
sunrise_sunset_get(
    sunrise_sunset_t *sun,
    sunrise_sunset_options_t *options
)
{
  struct addrinfo hints, *res, *res0;
  int error;
  int s;
  const char *host = "api.sunrise-sunset.org";
  const char *page_format = "json?lat=%f&lng=%f&formatted=%d&date=%s";
  const char *cause = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(host, "http", &hints, &res0);
  if (error) {
    errx(1, "%s", gai_strerror(error));
    /* NOTREACHED */
  }
  s = -1;
  for (res = res0; res; res = res->ai_next) {
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s < 0) {
      cause = "socket";
      continue;
    }

    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
      cause = "connect";
      close(s);
      s = -1;
      continue;
    }

    break;  /* okay we got one */
  }

  if (s < 0) {
    err(1, "%s", cause);
    /* NOTREACHED */
  } else {
    char *page = malloc(PAGE_MAX_BYTES);
    error = snprintf(page, PAGE_MAX_BYTES, page_format, options->latitude, options->longitude, 0, options->date);
    if (-1 == error) {
      fprintf(stderr, "malloc for page name failed\n");
      exit(EXIT_FAILURE);
    }

    char *recv_msg = malloc(RECV_MSG_MAX_BYTES );
    fail(!recv_msg, "malloc for recv_msg\n");
    get_page(s, host, page, recv_msg);
    free(page);

    int json_offset = get_json_in_page(recv_msg);
    fail(json_offset < 0, "Failed to fetch json\n");

    jsmn_parser jp;
    jsmn_init(&jp);
    jsmntok_t *tokens = malloc(MAX_TOKENS * sizeof(jsmntok_t));
    jsmn_parse(&jp, &recv_msg[json_offset], strnlen(&recv_msg[json_offset], PAGE_MAX_BYTES), tokens, MAX_TOKENS);

    get_dates(sun, &recv_msg[json_offset], tokens, jp.toknext);
    free(recv_msg);

  }
  freeaddrinfo(res0);

  return 0;
}
