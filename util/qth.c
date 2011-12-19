/* -*- c-basic-offset: 2; -*- */

/*
   qth: qth, locator, distance, and course computations

   Author: Dieter Deyke <dieter.deyke@gmail.com>
   Time-stamp: <2011-12-19 14:24:35 deyke>
*/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define CONFFILE        "/usr/local/lib/qth.conf"
#define RADIUS          6370.0

static char **argv;
static double mylatitude  = 49.008461;
static double mylongitude = -9.889875;

/*---------------------------------------------------------------------------*/

static void usage(void)
{
  printf("Usage:    qth <place> [<place>]\n");
  printf("              <place> ::= <locator>\n");
  printf("              <place> ::= <deg> [<min> [<sec>]] east|west\n");
  printf("                          <deg> [<min> [<sec>]] north|south\n");
  printf("\n");
  printf("Examples: qth jn48kp\n");
  printf("          qth 8 53 28 east 48 38 33 north\n");
  printf("          qth 9.9 east 49.1 north\n");
  printf("          qth jn48aa 9 east 48 30 north\n");
  exit(1);
}

/*---------------------------------------------------------------------------*/

static double safe_acos(double a)
{
  if (a >=  1.0) {
    return 0.0;
  }
  if (a <= -1.0) {
    return M_PI;
  }
  return acos(a);
}

/*---------------------------------------------------------------------------*/

static double norm_course(double a)
{
  while (a <    0.0) a += 360.0;
  while (a >= 360.0) a -= 360.0;
  return a;
}

/*---------------------------------------------------------------------------*/

static int double_to_char(double *dp, double step, int base_char)
{
  int dint = *dp / step;
  int c = dint + base_char;
  *dp = *dp - dint * step;
  return c;
}

/*---------------------------------------------------------------------------*/

static void deg_to_loc(double longitude, double latitude, char *loc)
{
  double step = 180.0;
  longitude = 180.0 - longitude;
  latitude  = latitude + 90.0;
  step /= 18;
  *loc++ = double_to_char(&longitude, 2 * step, 'A');
  *loc++ = double_to_char(&latitude,      step, 'A');
  step /= 10;
  *loc++ = double_to_char(&longitude, 2 * step, '0');
  *loc++ = double_to_char(&latitude,      step, '0');
  step /= 24;
  *loc++ = double_to_char(&longitude, 2 * step, 'a');
  *loc++ = double_to_char(&latitude,      step, 'a');
  step /= 10;
  *loc++ = double_to_char(&longitude, 2 * step, '0');
  *loc++ = double_to_char(&latitude,      step, '0');
  step /= 24;
  *loc++ = double_to_char(&longitude, 2 * step, 'a');
  *loc++ = double_to_char(&latitude,      step, 'a');
  *loc   = 0;
}

/*---------------------------------------------------------------------------*/

static double char_to_double(int c, double step, int base_char)
{
  if (c >= 'a' && c <= 'z') {
    c -= 32;
  }
  return step * (c - base_char);
}

/*---------------------------------------------------------------------------*/

static void loc_to_deg(const char *loc, double *longitude, double *latitude)
{

  double la = 0.0;
  double lo = 0.0;
  double step = 180.0;

  if (strlen(loc) >= 2) {
    step /= 18;
    lo += char_to_double(*loc++, 2 * step, 'A');
    la += char_to_double(*loc++,     step, 'A');
  }
  if (strlen(loc) >= 2) {
    step /= 10;
    lo += char_to_double(*loc++, 2 * step, '0');
    la += char_to_double(*loc++,     step, '0');
  }
  if (strlen(loc) >= 2) {
    step /= 24;
    lo += char_to_double(*loc++, 2 * step, 'A');
    la += char_to_double(*loc++,     step, 'A');
  }
  if (strlen(loc) >= 2) {
    step /= 10;
    lo += char_to_double(*loc++, 2 * step, '0');
    la += char_to_double(*loc++,     step, '0');
  }
  if (strlen(loc) >= 2) {
    step /= 24;
    lo += char_to_double(*loc++, 2 * step, 'A');
    la += char_to_double(*loc++,     step, 'A');
  }
  lo += step;     /* middle of field */
  la += step / 2; /* middle of field */
  *longitude = 180.0 - lo;
  *latitude = la - 90.0;
}

/*---------------------------------------------------------------------------*/

static const char *course_name(double a)
{
  if (a <=  11.25) return "North";
  if (a <=  33.75) return "North-North-East";
  if (a <=  56.25) return "North-East";
  if (a <=  78.75) return "East-North-East";
  if (a <= 101.25) return "East";
  if (a <= 123.75) return "East-South-East";
  if (a <= 146.25) return "South-East";
  if (a <= 168.75) return "South-South-East";
  if (a <= 191.25) return "South";
  if (a <= 213.75) return "South-South-West";
  if (a <= 236.25) return "South-West";
  if (a <= 258.75) return "West-South-West";
  if (a <= 281.25) return "West";
  if (a <= 303.75) return "West-North-West";
  if (a <= 326.25) return "North-West";
  if (a <= 348.75) return "North-North-West";
  if (a <= 371.25) return "North";
  return "???";
}

/*---------------------------------------------------------------------------*/

static void print_deg_min_sec(double d)
{

  int deg;
  int min;
  int sec;

  deg = (int) d;
  d = (d - deg) * 60;
  min = (int) d;
  d = (d - min) * 60;
  sec = (int) round(d);
  printf("%d %d' %d\"", deg, min, sec);
}

/*---------------------------------------------------------------------------*/

static void print_qth(const char *prompt, double longitude, double latitude, const char *loc)
{

  const char *plo;
  const char *pla;

  if (longitude < 0.0) {
    plo = "East";
    longitude = -longitude;
  } else {
    plo = "West";
  }
  if (latitude  < 0.0) {
    pla = "South";
    latitude = -latitude;
  } else {
    pla = "North";
  }
  printf("%s", prompt);
  print_deg_min_sec(longitude);
  printf(" %s ", plo);
  print_deg_min_sec(latitude);
  printf(" %s --> %s\n", pla, loc);
}

/*---------------------------------------------------------------------------*/

static int parse_arg(double *longitude, double *latitude)
{

  double step;
  int c;

  if (! *argv) return -1;

  if (isalpha(*argv[0])) {
    loc_to_deg(*argv, longitude, latitude);
    argv++;
    return 0;
  }

  *longitude = 0.0;
  step = 1.0;
  while (isdigit(*argv[0])) {
    *longitude += atof(*argv) * step;
    step /= 60;
    argv++;
  }
  if (! *argv) usage();
  c = *argv[0];
  if (c == 'E' || c == 'e') {
    *longitude = - *longitude;
  } else if (c != 'W' && c != 'w') {
    usage();
  }
  argv++;
  if (! *argv) usage();

  *latitude = 0.0;
  step = 1.0;
  while (isdigit(*argv[0])) {
    *latitude += atof(*argv) * step;
    step /= 60;
    argv++;
  }
  if (! *argv) usage();
  c = *argv[0];
  if (c == 'S' || c == 's') {
    *latitude = - *latitude;
  } else if (c != 'N' && c != 'n') {
    usage();
  }
  argv++;

  return 0;
}

/*---------------------------------------------------------------------------*/

int main(int pargc, char **pargv)
{

  FILE *fp;
  char loc1[11];
  char loc2[11];
  double a1;
  double a2;
  double b1;
  double b2;
  double e;
  double l1;
  double l2;
  double latitude1;
  double latitude2;
  double longitude1;
  double longitude2;
  int two_is_me;

  if ((fp = fopen(CONFFILE, "r"))) {
    if (fscanf(fp, "%lf %lf", &longitude1, &latitude1) == 2) {
      mylongitude = longitude1;
      mylatitude = latitude1;
    }
    fclose(fp);
  }

  argv = ++pargv;

  if (parse_arg(&longitude1, &latitude1)) usage();
  deg_to_loc(longitude1, latitude1, loc1);

  two_is_me = 0;
  if (parse_arg(&longitude2, &latitude2)) {
    longitude2 = mylongitude;
    latitude2  = mylatitude;
    two_is_me = 1;
  }
  deg_to_loc(longitude2, latitude2, loc2);

  if (*argv) usage();

  l1 = longitude1 / 180.0 * M_PI;
  l2 = longitude2 / 180.0 * M_PI;
  b1 = latitude1  / 180.0 * M_PI;
  b2 = latitude2  / 180.0 * M_PI;

  e = safe_acos(sin(b1) * sin(b2) + cos(b1) * cos(b2) * cos(l2-l1));

  if (!e)
    print_qth("qth:  ", longitude1, latitude1, loc1);
  else {
    if (two_is_me) {
      print_qth("your qth:       ", longitude1, latitude1, loc1);
      print_qth(" my  qth:       ", longitude2, latitude2, loc2);
    }
    else {
      print_qth("1st  qth:       ", longitude1, latitude1, loc1);
      print_qth("2nd  qth:       ", longitude2, latitude2, loc2);
    }

    printf("distance:       %.1f km = %.1f miles\n", e * RADIUS, e * RADIUS / 1.609344);

    a1 = atan2(sin(l1 - l2), cos(b1) * tan(b2) - sin(b1) * cos(l2 - l1)) / M_PI * 180.0;
    a2 = atan2(sin(l2 - l1), cos(b2) * tan(b1) - sin(b2) * cos(l1 - l2)) / M_PI * 180.0;

    a1 = norm_course(a1);
    a2 = norm_course(a2);

    if (two_is_me) {
      printf("course you->me: %3.0f (%s)\n", a1, course_name(a1));
      printf("course me->you: %3.0f (%s)\n", a2, course_name(a2));
    } else {
      printf("course 1 --> 2: %3.0f (%s)\n", a1, course_name(a1));
      printf("course 2 --> 1: %3.0f (%s)\n", a2, course_name(a2));
    }
  }
  return 0;
}
