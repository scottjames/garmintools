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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

// symlink compatibility mapper
struct garmin_compat_t_ {
  const char *name;
  int         argc;
  const char *argv[2];
};
typedef struct garmin_compat_t_ garmin_compat_t;

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

static garmin_compat_t compat[] = {
  {"garmin_save_runs", 1, {"download", NULL}},
  {"garmin_dump", 1, {"dump", NULL}},
  {"garmin_get_info", 1, {"info", NULL}},
  {"garmin_gmap", 2, {"convert", "--format=gmap"}},
  {"garmin_gchart", 2, {"convert", "--format=gchart"}},
  {"garmin_gpx", 2, {"convert", "--format=gpx"}},
  {"garmin_tcx", 2, {"convert", "--format=tcx"}},
  {NULL, 0, {NULL, NULL}},
};

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

static char **
build_argc_argv(int original_argc, char *original_argv[], int *argc)
{
  char **argv = NULL;

  // See if we are running from a compatibility symlink
  if (strstr(original_argv[0], "garmin_") != NULL) {
    garmin_compat_t *p = compat;
    for (; p->name != NULL; ++p) {
      if (strstr(original_argv[0], p->name) != NULL) {
        *argc = original_argc + p->argc - 1;
        argv  = calloc(*argc, sizeof(char *));
        for (int i = 0; i < p->argc; i++) {
          argv[i] = (char *)p->argv[i];
        }

        if (original_argc > 0) {
          memcpy(argv + p->argc,
                 original_argv + 1,
                 (original_argc - 1) * sizeof(char *));
        }

        break;
      }
    }
  }

  return argv;
}

int
main(int original_argc, char *original_argv[])
{
  char **argv      = NULL;
  int    argc      = 0;
  bool   free_argv = true;

  argv = build_argc_argv(original_argc, original_argv, &argc);

  if (argv == NULL) {
    free_argv = false;
    argv      = (char **)original_argv;
    argc      = original_argc;
    argv++;
    argc--;
  }

  int retval = 1;
  if (argc > 0) {
    if (strstr(argv[0], "--") == argv[0]) {
      argv[0] += 2;
    }
    retval = handle_command(argc, argv);
  } else {
    print_usage();
  }

  if (free_argv)
    free(argv);

  return retval;
}
