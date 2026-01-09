/*
 *  Copyright 2026 Patrick T. Head
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 *  @file dedup.c
 *  @brief cli command to dedup all indentical files from a directory compared
 *         to a different reference directory
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

#include "dedup.h"

#include "config.h"

static void usage(void);

  /**
   *  @fn int main(int argc, char **argv)
   *  @brief entry point for dedup CLI command
   *
   *  @param argc - number of command line arguments
   *  @param argv - array of command line argument strings
   *
   *  @return 0 on success, 1 on failure
   */

int main(int argc, char **argv)
{
  int c;
  int rv = 1;
  dedup_options options;

  memset(&options, 0, sizeof(dedup_options));

  while ((c = getopt(argc, argv, "r:w:dfqh")) != EOF)
  {
    switch (c)
    {
      case 'r':
        dedup_options_set_reference_directory(&options, optarg);
        break;

      case 'w':
        dedup_options_set_working_directory(&options, optarg);
        break;

      case 'd':
        dedup_options_set_remove_files(&options, true);
        break;

      case 'f':
        dedup_options_set_force_removal(&options, true);
        break;

      case 'q':
        dedup_options_set_quiet(&options, true);
        break;

      case 'h':
      default:
        usage();
        goto exit;
    }
  }

  if (!dedup_options_get_reference_directory(&options) ||
      !dedup_options_get_working_directory(&options))
  {
    usage();
    goto exit;
  }

  if (!strcmp(dedup_options_get_reference_directory(&options),
              dedup_options_get_working_directory(&options)))
  {
    fprintf(stderr, "reference directory must differ from working directory\n");
    goto exit;
  }

  if (dedup_options_get_remove_files(&options) &&
      !dedup_options_get_force_removal(&options))
  {
    printf("WARNING!!!!\n");
    printf("You are about to delete a possibly VERY large number of files from the system.\n");
    printf("Are you sure you want to proceed (y/N)?  ");
    fflush(stdout);
    c = getchar();
    c = toupper(c);
    if (c != 'Y') goto exit;
  }

  if (!dedup_options_get_quiet(&options))
  {
    printf("%-20.20s: %s\n",
           "reference directory",
           dedup_options_get_reference_directory(&options));
    printf("%-20.20s: %s\n",
           "working directory",
           dedup_options_get_working_directory(&options));
    printf("%-20.20s: %s\n",
           "remove files",
           dedup_options_get_remove_files(&options) ? "true" : "false");
    printf("%-20.20s: %s\n",
           "force removal",
           dedup_options_get_force_removal(&options) ? "true" : "false");
    printf("%-20.20s: %s\n",
           "quiet",
           dedup_options_get_quiet(&options) ? "true" : "false");
  }

  dedup(&options);

  rv = 0;

exit:
  return rv;
}

  /**
   *  @fn void usage(void)
   *  @brief prints usage message for dedup CLI command
   *
   *  @par Parameters
   *       None.
   *
   *  @par Returns
   *       Nothing.
   */

static void usage(void)
{
  printf("\n");
  printf("dedup version %s\n", VERSION);
  printf("\n");
  printf("  usage:\n");
  printf("\n");
  printf("    dedup [-d] [-f] [-q] -r <reference directory> -w <working directory>\n");
  printf("\n");
  printf("  where:\n");
  printf("\n");
  printf("    <reference directory> is root of directory to check for duplicates\n");
  printf("    <working directory> is root of directory where duplicates will be\n");
  printf("                               removed\n");
  printf("\n");
  printf("    -d -- required to actually remove duplicates, default: false\n");
  printf("\n");
  printf("    -f -- force; don't warn user before removal, default: false\n");
  printf("\n");
  printf("    -q -- quiet; don't display options or progress, default: false\n");
  printf("\n");
  printf("    -h -- this help screen\n");
  printf("\n");
  printf("  NOTE:  It is highly recommended to run dedup without the -d option first,\n");
  printf("         then verify that all files listed for removal are OK to remove,\n");
  printf("         then re-run dedup with the -d option\n");
  printf("\n");
  printf("  WARNING:  Once a file is removed by dedup it will be impossible or very\n");
  printf("            difficult to recover that file.\n");
  printf("\n");
}

