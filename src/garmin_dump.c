/*
  Garmintools software package
  Copyright (C) 2006-2008 Dave Bailey

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

#include "garmin.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verbose = 0;

static void
print_usage(const char *name)
{
  fprintf(stderr, "Usage : %s [OPTIONS] FILE ...\n", name);
  fprintf(stderr, "\nDump gmn files to stdout in human-readable format\n");
  fprintf(stderr, "  -h, --help    Provide help\n");
  fprintf(stderr, "  -v, --verbose Be more verbose\n");
}

int
garmin_dump ( int argc, const char ** argv )
{
  garmin_data * data;
  int           i;

  static struct option options[] = {{"help", no_argument, 0, 'h'},
                                    {"verbose", no_argument, &verbose, 1},
                                    {0, 0, 0, 0}};

  while (true) {
    int option_index = -1;
    int c =
      getopt_long(argc, (char *const *)argv, "hv", options, &option_index);
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
    default:
      print_usage(argv[0]);
      exit(c == 'h' ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  }

  if (argc < 2) {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1], "help") == 0) {
    print_usage(argv[0]);
    exit(EXIT_SUCCESS);
  }

  printf("<?xml version=\"1.0\"?>\n");
  printf("<garmin>\n");
  for ( i = 1; i < argc; i++ ) {
    if ( (data = garmin_load(argv[i])) != NULL ) {
      printf("<activity>\n");
      garmin_print_data(data,stdout,0);
      printf("</activity>\n");
      garmin_free_data(data);
    }
  }
  printf("</garmin>\n");

  return EXIT_SUCCESS;
}
