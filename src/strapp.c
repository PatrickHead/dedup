/*
 *  Copyright 2021 Patrick T. Head
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file strapp.c
 *  @brief A function to auto allocate and append a string to an existing string.
 */

#include <stdlib.h>
#include <string.h>

#include "strapp.h"

#include "config.h"

 /**
  * @fn char *strapp(char *s1, char *s2)
  *
  * @brief appends @p s2 to @p s1 , reallocating memory
  *
  * NOTE:  The user is responsible for freeing @p s2 if required.
  *
  * @param s1 - original string, can be NULL
  * @param s2 - addendum to string
  *
  * @return pointer to newly formed string
  */

char *strapp(char *s1, char *s2)
{
  int len1, len2;
  char *tmp;

  if (!s1)
  {
    s1 = (char *)malloc(1);
    if (!s1) return NULL;
    *s1 = 0;
  }

  if (!s2) return s1;

  len1 = strlen(s1);
  len2 = strlen(s2);

  tmp = (char *)realloc(s1, len1 + len2 + 1);
  if (tmp)
  {
    s1 = tmp;
    strcat(s1, s2);
  }

  return s1;
}

