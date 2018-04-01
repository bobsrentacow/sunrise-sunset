#ifndef __SUNRISE_SUNSET_H__
 #define __SUNRISE_SUNSET_H__

#include <time.h>

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

typedef enum keywords {
  SUNRISE                    ,
  SUNSET                     ,
  SOLAR_NOON                 ,
  CIVIL_TWILIGHT_BEGIN       ,
  CIVIL_TWILIGHT_END         ,
  NAUTICAL_TWILIGHT_BEGIN    ,
  NAUTICAL_TWILIGHT_END      ,
  ASTRONOMICAL_TWILIGHT_BEGIN,
  ASTRONOMICAL_TWILIGHT_END 
} keywords_e;

typedef struct sunrise_sunset {
  int day_length;
  struct tm times[sizeof(date_keywords)/sizeof(date_keywords[0])];
} sunrise_sunset_t;

typedef struct ss_options {
  double latitude;
  double longitude;
  char *date;
} sunrise_sunset_options_t;

extern int
sunrise_sunset_get(
    sunrise_sunset_t *sun,
    sunrise_sunset_options_t *options
);

#endif //__SUNRISE_SUNSET_H__
