/*
  Garmintools software package
  Copyright (C) 2006-2008 Dave Bailey
  Copyright (C) 2019 Jens Georg

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int
garmin_dump(int argc, char *argv[]);

extern int
garmin_tcx(int argc, char *argv[], const char *output_file, bool verbose);

extern int
garmin_gchart(int argc, char *argv[], const char *output_file, bool verbose);

extern int
garmin_gpx(int argc, char *argv[], const char *output_file, bool verbose);

extern int
garmin_gmap(int argc, char *argv[], const char *output_file, bool verbose);

static int verbose = 0;

static void
print_usage(const char *name)
{
  fprintf(stderr, "Usage: %s [-v] -f FORMAT [-o FILE] FILE...\n\n", name);
  fprintf(stderr,
          "  -f, --format: Output format to convert to. Can be one of\n");
  fprintf(
    stderr,
    "                dump, gpx, tcx, gmap, gchart. Default is \"dump\"\n");
  fprintf(stderr, "  -o, --output: Name of the file to write the output to\n");
}

typedef enum {
  GARMIN_OUTPUT_FORMAT_NONE,
  GARMIN_OUTPUT_FORMAT_DUMP,
  GARMIN_OUTPUT_FORMAT_TCX,
  GARMIN_OUTPUT_FORMAT_GPX,
  GARMIN_OUTPUT_FORMAT_GMAP,
  GARMIN_OUTPUT_FORMAT_GCHART
} garmin_output_format_t;

char *const format_list[] = {[GARMIN_OUTPUT_FORMAT_NONE]   = "none",
                             [GARMIN_OUTPUT_FORMAT_DUMP]   = "dump",
                             [GARMIN_OUTPUT_FORMAT_TCX]    = "tcx",
                             [GARMIN_OUTPUT_FORMAT_GPX]    = "gpx",
                             [GARMIN_OUTPUT_FORMAT_GMAP]   = "gmap",
                             [GARMIN_OUTPUT_FORMAT_GCHART] = "gchart"};

int
garmin_convert(int argc, char *argv[], const char *output_file)
{
  garmin_output_format_t format = GARMIN_OUTPUT_FORMAT_DUMP;

  static struct option options[] = {{"verbose", no_argument, &verbose, 1},
                                    {"format", required_argument, 0, 'f'},
                                    {0, 0, 0, 0}};

  while (true) {
    int option_index = -1;
    int c            = getopt_long(argc, argv, ":hvf:", options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (options[option_index].flag != 0) {
        break;
      }
      break;
    case 'v':
      verbose = 1;
      break;
    case 'f':
      if (strncasecmp("dump", optarg, 4) == 0) {
        format = GARMIN_OUTPUT_FORMAT_DUMP;
      } else if (strncasecmp("tcx", optarg, 3) == 0) {
        format = GARMIN_OUTPUT_FORMAT_TCX;
      } else if (strncasecmp("gpx", optarg, 3) == 0) {
        format = GARMIN_OUTPUT_FORMAT_GPX;
      } else if (strncasecmp("gmap", optarg, 4) == 0) {
        format = GARMIN_OUTPUT_FORMAT_GMAP;
      } else if (strncasecmp("gchart", optarg, 6) == 0) {
        format = GARMIN_OUTPUT_FORMAT_GCHART;
      } else {
        fprintf(stderr, "Invalid output format specified: %s\n", optarg);
        print_usage(argv[0]);
        return EXIT_FAILURE;
      }
      break;
    case 'o':
      output_file = optarg;
      break;
    case '?':
      // Ignore all unknown options, we will pass them on to the subcommand
      break;
    default:
      print_usage(argv[0]);
      return (c == 'h' ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  }

  int    offset   = optind - 1;
  int    new_argc = argc - offset;
  char **new_argv = argv + offset;

  printf("=optind: %d\n", optind);
  optind = 1;

  for (int i = 0; i < new_argc; i++) {
    printf("%d: %s\n", i, new_argv[i]);
  }

  switch (format) {
  case GARMIN_OUTPUT_FORMAT_DUMP:
    return garmin_dump(new_argc, new_argv);
  case GARMIN_OUTPUT_FORMAT_TCX:
    return garmin_tcx(new_argc, new_argv, output_file, verbose);
  case GARMIN_OUTPUT_FORMAT_GCHART:
    return garmin_gchart(new_argc, new_argv, output_file, verbose);
  case GARMIN_OUTPUT_FORMAT_GPX:
    return garmin_gpx(new_argc, new_argv, output_file, verbose);
  case GARMIN_OUTPUT_FORMAT_GMAP:
    return garmin_gmap(new_argc, new_argv, output_file, verbose);
  default:
    fprintf(stderr, "%s: Not yet implemented\n", format_list[format]);
    break;
  }

  return EXIT_SUCCESS;
}
