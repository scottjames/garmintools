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

#include <stdio.h>
#include <unistd.h>
#include "garmin.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

static int verbose = 0;

static void
print_usage(const char *name)
{
  fprintf(stderr, "Usage : %s [OPTIONS]\n", name);
  fprintf(stderr, "\nDownload excercise information from the device\n");
  fprintf(stderr, "  -h, --help    Provide help\n");
  fprintf(stderr, "  -v, --verbose Be more verbose\n");
}

int
garmin_download(int argc, char **argv)
{
  garmin_unit garmin;

  static struct option options[] = {{"help", no_argument, 0, 'h'},
                                    {"verbose", no_argument, &verbose, 1},
                                    {0, 0, 0, 0}};

  while (true) {
    int option_index = -1;
    int c            = getopt_long(argc, argv, "hv", options, &option_index);
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

  if (argc > 1 && strcmp(argv[1], "help") == 0) {
    print_usage(argv[0]);
    exit(EXIT_SUCCESS);
  }

  if ( garmin_init(&garmin,verbose) != 0 ) {
    /* Read and save the runs. */
    garmin_save_runs(&garmin);

    garmin_close (&garmin);
    garmin_shutdown (&garmin);
  } else {
    printf("garmin unit could not be opened!\n");
  }

  return 0;
}
