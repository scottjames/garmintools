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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Replace with proper gettext later
#define N_(x) (x)
#define _(x) (x)

// Sub-command call-back function prototype
typedef int (*garmin_command_t)(int, char *argv[]);

// Sub-command dispatch table
struct garmin_command_dispatch_t_ {
    const char *name;
    garmin_command_t callback;
    const char *description;
};
typedef struct garmin_command_dispatch_t_ garmin_command_dispatch_t;

//////////////////////////////////////
// Function prototypes
//////////////////////////////////////

// External prototypes
extern int
garmin_info(int argc, char *argv[]);
extern int
garmin_dump(int argc, char *argv[]);
extern int
garmin_download(int argc, char *argv[]);
extern int
garmin_convert(int argc, char *argv[]);

// Internal command prototypes
static int
garmin_version(int argc, char *argv[]);
static int
garmin_help(int argc, char *argv[]);

// Misc. prototypes
static void
print_usage();
static int
handle_command(int argc, char *argv[]);

static garmin_command_dispatch_t commands[] = {
  {"help", garmin_help, N_("Show help for the various subcommands")},
  {"version", garmin_version, N_("Show the version of the garmintool")},
  {"download", garmin_download, N_("Get recordings from the connected device")},
  {"convert",
   garmin_convert,
   N_("Convert binary excercise dumps to various output formats")},
  {"dump", garmin_dump, N_("Dump gmn files to human-readable pseudo-XML")},
  {"info", garmin_info, N_("Dump information from the connected device")},
  {NULL, NULL, NULL}};

static int
handle_command(int argc, char *argv[])
{
  garmin_command_dispatch_t *p = commands;
  for (; p->name != NULL; ++p) {
    if (strstr(p->name, argv[0]) != p->name) {
      continue;
    }

    return p->callback(argc, argv);
  }

  fprintf(stderr,
          _("\"%s\" is not a garmintool command. See garmintool --help for "
            "possible commands\n"),
          argv[0]);

  return EXIT_FAILURE;
}

static void
print_usage() {
    printf(_("The following commands are available:\n\n"));

    garmin_command_dispatch_t *p = commands;
    while (p->name != NULL) {
      printf("      %8s : %s\n", p->name, _(p->description));
      p++;
    }
}

static int
garmin_version(int argc, char *argv[])
{
  printf("garmintool v%s\n", GARMINTOOLS_VERSION_STRING);

  return EXIT_SUCCESS;
}

static int
garmin_help(int argc, char *argv[])
{
  if (argc == 1) {
    print_usage();

    return EXIT_SUCCESS;
  }

  // Switch the two args, let the subcommand handle the help
  argv[0] = argv[1];
  argv[1] = "--help";

  return handle_command(argc, argv);
}

int
main(int original_argc, char *original_argv[])
{
  char **argv = (char **)original_argv;
  int    argc = original_argc;

  argv++;
  argc--;

  if (argc > 0) {
    if (strstr(argv[0], "--") == argv[0]) {
      argv[0] += 2;
    }
    } else {
        print_usage();
        exit (1);
    }

    return handle_command(argc, argv);
}
