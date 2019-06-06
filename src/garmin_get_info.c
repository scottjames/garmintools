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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int verbose = 0;

void
print_usage(const char *name)
{
  fprintf(stderr, "Usage : %s [OPTIONS]", name);
  fprintf(stderr, "\nShow information about the connected device\n");
  fprintf(stderr, "  -h, --help    Provide help\n");
  fprintf(stderr, "  -v, --verbose Be more verbose\n");
}

int
main ( int argc, char ** argv )
{
  garmin_unit   garmin;

  /* Set the verbosity if the -v option was provided. */

  static struct option options[] = {{"help", no_argument, 0, 'h'},
                                    {"verbose", no_argument, &verbose, 1},
                                    {0, 0, 0, 0}};

  while (true) {
    int c = getopt_long(argc, argv, "hv", options, NULL);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      break;
    case 'v':
      verbose = 1;
      break;
    default:
      print_usage(argv[0]);
      exit(c == 'h' ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  }

  if (argc > 1 && (strcmp(argv[1], "help") == 0)) {
    print_usage(argv[0]);
    exit(EXIT_SUCCESS);
  }

  if ( garmin_init(&garmin,verbose) != 0 ) {
    /* Now print the info. */
    garmin_print_info(&garmin,stdout,0);
    garmin_close (&garmin);
    garmin_shutdown (&garmin);
  } else {
    fprintf(stderr, "garmin unit could not be opened!\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
