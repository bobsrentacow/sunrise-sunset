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
#include "math.h"

static void
display_dates(
    sunrise_sunset_t *sun
)
{
  const char day_length_keyword[] = "day_length";
  const char *date_keywords[] = {
    "sunrise"                    ,
    "sunset"                     ,
    "solar_noon"                 ,
    "civil_twilight_begin"       ,
    "civil_twilight_end"         ,
    "nautical_twilight_begin"    ,
    "nautical_twilight_end"      ,
    "astronomical_twilight_begin",
    "astronomical_twilight_end"
  };

  time_t now;
  time(&now);
  int secs_until;
  char calstr[30];

  int ii;
  for (ii=0; ii<sizeof(date_keywords)/sizeof(date_keywords[0]); ii++) {
    ctime_r(&sun->times[ii], calstr);
    calstr[strnlen(calstr, sizeof(calstr)/sizeof(calstr[0]))-1] = '\0';

    secs_until = (int) nearbyint(difftime(now, sun->times[ii]));

    printf("%-27s: %-30s (%5d seconds%s)\n", date_keywords[ii], calstr, abs(secs_until), (secs_until < 0) ? "" : " ago");
  }
  printf("%-27s: %d", day_length_keyword, sun->day_length);
}

static void
usage(
    char *argv0
)
{
  fprintf(stderr, "Usage: %s \n"
    "-h (--help)      - this information\n"
    "-d (--date)      - date (default today)\n"
    "      acceptable formats:\n"
    "        YYYY-MM-DD\n"
    "        Other php date formats:\n"
    "          http://php.net/manual/en/datetime.formats.date.php\n"
    "        Relative php date formats:\n"
    "           http://php.net/manual/en/datetime.formats.relative.php\n"
    "-x (--longitude) - matrix width (default 8)\n"
    "-y (--latitude)  - matrix height (default 8)\n"
    , argv0);
  exit(-1);
}

static void
parse_arguments(
    sunrise_sunset_options_t *ss_options,
    int argc,
    char *argv[]
)
{

  // if 3rd field is nonzero, it is int* to a flag. int value of 4th field is stored in 3rd field ptr and returns 0
  // if 3rd field is zero, int value of 4th field is returned
  static struct option longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"file", required_argument, 0, 'f'},
    {"date", required_argument, 0, 'd'},
    {"latitude", required_argument, 0, 'y'},
    {"longitude", required_argument, 0, 'x'},
    {0, 0, 0, 0}
  };

  opterr = true; // ask getopt_long to display an error if a required arg isn't found
  //optind <-- index of next element of argv to be processed
  //optopt <-- unknown options and known ones with missing required arg are stored here
  //optarg <-- argument of the option will be here if it exists
  int index; // index of longopts[] when a long option is identified
  int c; // return from getopt_long which is:
         //  0 when int *flag (3rd field) of struct option is nonzero for this long option
         //  -1 for no more options
         //  the option character for short options
         //  the value (4th field)

  while (true) {
    index = 0;
    c = getopt_long(argc, argv, "hf:d:x:y:", longopts, &index);
    if (c == -1)
      break;

    switch (c) {
      case 0: // long option with a nonzero 3rd field in struct option
        break;

      case 'h':
        usage(argv[0]);

      case 'd':
        if (!optarg)
          usage(argv[0]);
        asprintf(&ss_options->date, "%s", optarg);
        break;

      case 'x':
        if (!optarg)
          usage(argv[0]);
        ss_options->longitude = strtod(optarg, NULL);
        break;

      case 'y':
        if (!optarg)
          usage(argv[0]);
        ss_options->latitude = strtod(optarg, NULL);
        break;

      case '?':
        exit(-1);

      default:
        exit(-1);
    }
  }
}

int
main(
    int argc,
    char *argv[]
)
{
  sunrise_sunset_options_t options;

  char calstr[30];
  time_t now;
  struct tm bdt;
  time(&now);
  localtime_r(&now, &bdt);
  strftime(calstr, sizeof(calstr)/sizeof(calstr[0]), "%F", &bdt);

  options.date = calstr;
  options.latitude = 44.907366;
  options.longitude = -92.968065;

  parse_arguments(&options, argc, argv);

  fprintf(stderr, "date     : %s\n", options.date);
  fprintf(stderr, "latitude : %f\n", options.latitude);
  fprintf(stderr, "longitude: %f\n", options.longitude);

  sunrise_sunset_t sun;
  sunrise_sunset_get(&sun, &options);
  display_dates(&sun);

  return 0;
}

