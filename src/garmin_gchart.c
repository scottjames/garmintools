#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "garmin.h"

#include <getopt.h>
#include <stdbool.h>

/* Constants */
#define SIMP_ENC 's'
#define EXT_ENC  'e'
#define TXT_ENC  't'

/* Default Values */
#define DEF_WIDTH       1000
#define DEF_HEIGHT      300
#define DEF_PIXPERDP    499.0
#define DEF_ENC_METHOD  EXT_ENC

/* Type Defs */
typedef struct d304strs {
  char*  time;
  int    time_len;
  char*  alt;
  int    alt_len;
  char*  distance;
  int    distance_len;
  char*  heart_rate;
  int    heart_rate_len;
  char*  cadence;
  int    cadence_len;
} d304strs;

typedef struct D304_ext {
  position_type       posn;
  unsigned long long  time;
  float32             alt;
  float32             distance;
  uint8               heart_rate;
  uint8               cadence;
  bool                sensor;
} D304_ext;

typedef struct gchart_conf {
  int      width;
  int      height;
  char     encode_method;
  float32  pixperdp;
} gchart_conf;


/* Functions */

static float gchart_t_encode(float32 num, float32 max) {
    return (floor(1000*num/max)/10);
}


static char gchart_e_encode_single(int num) {
    if (num < 0 )
        return '_';
    if (num <= 25 )
        return 'A' + num;
    if (num <= 51 )
        return 'a' + num - 26;
    if (num <= 61 )
        return '0' + num - 52;
    if (num == 62 )
        return '-';
    if (num == 63 )
        return '.';
    return '_';
}


static void
gchart_e_encode ( float32 num, float32 max, char * str )
{
  /* printf("-- encoding %f of %f => ", num, max); */
  num = 4095*num/max;
  /* printf(" %f\n", num); */
  if (num < 0 || num > 4095 ) {
    str[0] = '_';
    str[1] = '_';
    str[2]= '\0';
  } else {
    str[0]=gchart_e_encode_single((int)num/64);
    str[1]=gchart_e_encode_single(num - ((int)(num/64)*64) );
    str[2]='\0';
  }
}


static void
get_gchart_max_data ( garmin_data *  data,
                      D304 *         max,
                      int *          datapoints_num,
                      uint32 *       min_time )
{
  garmin_list *       dlist;
  garmin_list_node *  node;
  garmin_data *       point;
  D304 *              d304;

  max->time = 0;
  max->distance = 0;
  max->alt = 0;
  max->heart_rate = 0;
  max->cadence = 0;

  if ( data != NULL ) {
    data = garmin_list_data(data,2);
    if ( data->type == data_Dlist ) {
      dlist = data->data;

      (*datapoints_num)=0;
      for ( node = dlist->head; node != NULL; node = node->next ) {
        point = node->data;
        if ( point->type == data_D304 ) {
          (*datapoints_num)++;
          d304 = point->data;
          if ( d304->distance > max->distance && d304->distance < 1.0e24 )
            max->distance = d304->distance;
          if ( d304->alt > max->alt && d304->alt < 1.0e24)
            max->alt = d304->alt;
          if ( d304->time > max->time ) max->time = d304->time;
          if ( d304->time < (*min_time) ) (*min_time) = d304->time;
          if ( d304->heart_rate > max->heart_rate )
            max->heart_rate = d304->heart_rate;
          if ( d304->cadence > max->cadence ) max->cadence = d304->cadence;
        }
      }
    }
  }
}


static int
get_gchart_data ( garmin_data *  data,
                  gchart_conf *  conf,
                  D304 *         max,
                  int            datapoints_num,
                  d304strs*      encoded_strings,
                  int            min_time )
{
  garmin_list *       dlist;
  garmin_list_node *  node;
  garmin_data *       point;
  D304 *              d304;
  D304_ext            total;
  int           ok     = 0;
  int j = 0;
  int i = 0;
  int np = 0;

  total.time = 0;
  total.distance = 0;
  total.alt = 0;
  total.heart_rate = 0;
  total.cadence = 0;

  if ( data != NULL ) {
    data = garmin_list_data(data,2);
    if ( data->type == data_Dlist ) {
      dlist = data->data;

      np = (int)ceil(datapoints_num / (conf->width / conf->pixperdp));
      i=1;

      /* Add check to make sure we won't overflow our string length */

      /* printf("summarizing: "); */
      for ( node = dlist->head; node != NULL; node = node->next ) {
        point = node->data;
        if ( point->type == data_D304 ) {

          d304 = point->data;

          total.time += d304->time;
          /* printf("--------- %d %d %lld\n", d304->time, j, total.time); */
          total.distance += d304->distance;
          total.alt += d304->alt;
#if 0
          total.heart_rate += d304->heart_rate;
          total.cadence += d304->cadence;
#endif

          if ( d304->posn.lat == 0x7fffffff && d304->posn.lon == 0x7fffffff )
            continue;

          if ( ++j % np == 0 ) {
            char str[3] = { 0 };
            int d=(total.distance == 0) ? 0 : (int)total.distance/np;
            int a=(total.alt == 0) ? 0 : (int)total.alt/np;
            int t=(total.time == 0) ? 0 : (int)(total.time/np);
#if 0
            /* printf("--------- %d -- np: %d\n", t, np); */
            int h=(total.heart_rate == 0) ? 0 : total.heart_rate/np;
            int c=(total.cadence == 0) ? 0 : total.cadence/np;
#endif
            switch (conf->encode_method) {
            case EXT_ENC:
              gchart_e_encode(t - min_time, max->time - min_time, str);
              encoded_strings->time_len +=
                sprintf(encoded_strings->time + encoded_strings->time_len, "%s", str);

              gchart_e_encode(d, max->distance, str);
              encoded_strings->distance_len +=
                sprintf(encoded_strings->distance + encoded_strings->distance_len, "%s", str);

              gchart_e_encode(a, max->alt, str);
              encoded_strings->alt_len +=
                sprintf(encoded_strings->alt + encoded_strings->alt_len, "%s", str);
              break;
            case TXT_ENC:
              /* printf("-- Encoding time: %d (%d) of %d (%d)\n", t, t-min_time, max->time, max->time - min_time); */
              encoded_strings->time_len +=
                sprintf(encoded_strings->time + encoded_strings->time_len, "%.1f,",
                        gchart_t_encode(t- min_time, max->time - min_time));

              encoded_strings->distance_len +=
                sprintf(encoded_strings->distance + encoded_strings->distance_len, "%.1f,",
                        gchart_t_encode(d, max->distance));

              encoded_strings->alt_len +=
                sprintf(encoded_strings->alt + encoded_strings->alt_len, "%.1f,",
                        gchart_t_encode(a, max->alt));
              break;
        default:
          break;
            }

            /* printf(" = %.1d\n", d); */
            /* printf("summarizing: "); */
            total.time = 0;
            total.distance = 0;
            total.alt = 0;
            total.heart_rate = 0;
            total.cadence = 0;
            j=0;
          }
          /* printf("%.1f, ", d304->distance); */

          i++;
        }
      }
      if (conf->encode_method == TXT_ENC) {
        /* Remove the trailing commas */
        encoded_strings->time[encoded_strings->time_len - 1]='\0';
        encoded_strings->distance[encoded_strings->distance_len - 1]='\0';
        encoded_strings->alt[encoded_strings->alt_len - 1]='\0';
#if 0
        encoded_strings->heart_rate[encoded_strings->heart_rate_len - 1]='\0';
        encoded_strings->cadence[encoded_strings->cadence_len - 1]='\0';
#endif
      }
      ok = 1;
    }
  }

  return ok;
}


static void
print_gchart_data ( garmin_data *  data,
                    FILE *         fp,
                    gchart_conf *  conf)
{
  D304 max;
  uint32 min_time=999999999;
  d304strs enc_strs = {0};
  int datapoints_num=0;
  char str_d[1024] = {0};
  char str_a[1024] = {0};
  char str_t[1024] = {0};

  enc_strs.distance = str_d;
  enc_strs.distance_len = 0;
  enc_strs.alt = str_a;
  enc_strs.alt_len = 0;
  enc_strs.time = str_t;
  enc_strs.time_len = 0;

  get_gchart_max_data ( data, &max, &datapoints_num, &min_time);
  /*
    fprintf(fp, "-- max_d: %f\n", max.distance);
    fprintf(fp, "-- max_a: %f\n", max.alt);
    fprintf(fp, "-- max_t: %d\n", max.time);
    fprintf(fp, "-- min_t: %d\n", min_time);
  */

  get_gchart_data(data, conf, &max, datapoints_num, &enc_strs, min_time);

  /* Distance vs Alt */
  fprintf(fp, "http://chart.apis.google.com/chart?cht=lxy&chtt=Distance+vs.+Alt&chs=%dx%d&chd=", conf->width, conf->height);
  switch (conf->encode_method) {
  case TXT_ENC: fprintf(fp, "t:%s|%s\n", enc_strs.distance, enc_strs.alt); break;
  case EXT_ENC: fprintf(fp, "e:%s,%s\n", enc_strs.distance, enc_strs.alt); break;
  default: break;
  }
  /* Time vs Distance */
  fprintf(fp, "http://chart.apis.google.com/chart?cht=lxy&chtt=Time+vs.+Distance&chs=%dx%d&chd=", conf->width, conf->height);
  switch (conf->encode_method) {
  case TXT_ENC: fprintf(fp, "t:%s|%s\n", enc_strs.time, enc_strs.distance); break;
  case EXT_ENC: fprintf(fp, "e:%s,%s\n", enc_strs.time, enc_strs.distance); break;
  default: break;
  }
}

static int verbose = 0;

static void
print_usage(const char *name)
{
  fprintf(stderr, "Usage: %s [OPTIONS] FILE ...\n", name);
  fprintf(stderr,
          "  -w, --width=WIDTH            Width of the graph\n"
          "  -h, --height=HEIGHT          Height of the graph\n"
          "  -p, --pix-per-data-point=PIX Pixels per datapoint\n"
          "  -e, --encode-method=[e|t]    Value encoding method. (t)ext or "
          "(e)xtended\n"
          "\n"
          "Legacy options:\n"
          "  -pdbp is equivalent as --pix-per-data-point\n"
          "  -enc is equivalent to --encode-method\n");
}

int
garmin_gchart(int argc, char **argv)
{
  int i=0;
  garmin_data * data;
  gchart_conf conf;

  /* Set the defaults */

  conf.width=DEF_WIDTH;
  conf.height=DEF_HEIGHT;
  conf.encode_method=DEF_ENC_METHOD;
  conf.pixperdp=DEF_PIXPERDP;

  static struct option options[] = {
    {"width", required_argument, 0, 'w'},
    {"height", required_argument, 0, 'h'},
    {"pix-per-data-point", required_argument, 0, 'p'},
    {"pdbp", required_argument, 0, 'p'},
    {"encode-method", required_argument, 0, 'e'},
    {"enc", required_argument, 0, 'e'},
    {"help", no_argument, 0, '?'},
    {"verbose", no_argument, &verbose, 'v'},
    {0, 0, 0, 0},
  };

  optind = 0;
  while (true) {
    int c = getopt_long_only(argc, argv, "w:h:p:e:", options, NULL);
    if (c == -1) {
      break;
    }

    switch (c) {
    case 'w':
      conf.width = (int)strtoul(optarg, NULL, 10);
      break;
    case 'h':
      conf.height = (int)strtoul(optarg, NULL, 10);
      break;
    case 'e':
      conf.encode_method = optarg[0];
      break;
    case 'p':
      conf.pixperdp = strtof(optarg, NULL);
      break;
    case '?':
      printf(">>%c\n", optopt);
      break;
    default:
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  };

  if (argc < 2) {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1], "help") == 0) {
    print_usage(argv[0]);
    exit(EXIT_SUCCESS);
  }

  printf("Resolution: %dx%d\nEncoding method: %c\nPixel per dp: %f\n",
         conf.width,
         conf.height,
         conf.encode_method,
         conf.pixperdp);

  for (i = optind; i < argc; i++) {
    if ( (data = garmin_load(argv[i])) != NULL ) {
      print_gchart_data(data,stdout,&conf);
      garmin_free_data(data);
    }
  }

  return 0;
}
