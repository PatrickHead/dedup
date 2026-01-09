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
 *  @file libdedup.c
 *  @brief library to dedup all indentical files from a directory compared to a
 *         different reference directory
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "strapp.h"
#include "avl.h"

#include "dedup.h"

#include "config.h"

static char *uint8_to_hex(uint8_t c);
static void build_root(dedup_hashes *root, char *root_path);
static void build_list(dedup_list *list, char *root_path);

  /**
   *  @fn void dedup(dedup_options *options)
   *  @brief runs deduplication process
   *
   *  @param options - pointer to dedup options to use during process
   *
   *  @par Returns
   *       Nothing.
   */

void dedup(dedup_options *options)
{
  dedup_hashes *root = NULL;
  dedup_list *list = NULL;
  dedup_item *item = NULL;
  dedup_node *found;
  char *message = NULL;

  if (!options) goto exit;

  root = dedup_hashes_build(dedup_options_get_reference_directory(options));
  if (!root) goto exit;

  list = dedup_list_build(dedup_options_get_working_directory(options));
  if (!list) goto exit;

  for (item = dedup_list_head(list); item; item = dedup_list_next(list))
  {
    message = NULL;
    found = dedup_hashes_find_hash(root, item->hash);
    if (found)
    {
      if (dedup_options_get_remove_files(options))
      {
        unlink(item->pathname);
        message = dedup_item_status(true, item->pathname, errno);
      }
      else if (!dedup_options_get_quiet(options))
        message = dedup_item_status(false, item->pathname, 0);

      if (message)
      {
        dedup_display_status(message, NULL);
        free(message);
        message = NULL;
      }
    }
  }

exit:
  if (root) dedup_hashes_free(root);
  if (list) dedup_list_free(list);
}

  /**
   *  @fn dedup_options *dedup_options_new(void)
   *  @brief creates a new @a dedup_options struct
   *
   *  @par Parameters
   *       None.
   *
   *  @return pointer to new @a dedup_options struct
   */

dedup_options *dedup_options_new(void)
{
  dedup_options *options = NULL;

  options = malloc(sizeof(dedup_options));
  if (options) memset(options, 0, sizeof(dedup_options));

  return options;
}

  /**
   *  @fn void dedup_options_free(dedup_options *options)
   *  @brief frees memory allocated to @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_options_free(dedup_options *options)
{
  if (!options) return;

  if (options->reference_directory) free(options->reference_directory);
  if (options->working_directory) free(options->working_directory);
  free(options);
}

  /**
   *  @fn bool dedup_options_get_remove_files(dedup_options *options)
   *  @brief returns value of remove_files flag from @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *
   *  @return bool - true if enable, false if disabled
   */

bool dedup_options_get_remove_files(dedup_options *options)
{
  return options ? options->remove_files : false;
}

  /**
   *  @fn void dedup_options_set_remove_files(dedup_options *options,
   *                                          bool remove_files)
   *  @brief sets value of remove_files flag in @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *  @param remove_files - true to enable, false to disable
   *
   *  @par Returns
   *       Nothing
   */

void dedup_options_set_remove_files(dedup_options *options, bool remove_files)
{
  if (!options) return;
  options->remove_files = remove_files;
}

  /**
   *  @fn bool dedup_options_get_force_removal(dedup_options *options)
   *  @brief returns value of force_removal flag from @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *
   *  @return bool - true if enable, false if disabled
   */

bool dedup_options_get_force_removal(dedup_options *options)
{
  return options ? options->force_removal : false;
}

  /**
   *  @fn void dedup_options_set_force_removal(dedup_options *options,
   *                                           bool force_removal)
   *  @brief sets value of force_removal flag in @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *  @param force_removal - true to enable, false to disable
   *
   *  @par Returns
   *       Nothing
   */

void dedup_options_set_force_removal(dedup_options *options, bool force_removal)
{
  if (!options) return;
  options->force_removal = force_removal;
}

  /**
   *  @fn bool dedup_options_get_quiet(dedup_options *options)
   *  @brief returns value of quiet flag from @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *
   *  @return bool - true if enable, false if disabled
   */

bool dedup_options_get_quiet(dedup_options *options)
{
  return options ? options->quiet : false;
}

  /**
   *  @fn void dedup_options_set_quiet(dedup_options *options, bool quiet)
   *  @brief sets value of quiet flag in @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *  @param quiet - true to enable, false to disable
   *
   *  @par Returns
   *       Nothing
   */

void dedup_options_set_quiet(dedup_options *options, bool quiet)
{
  if (!options) return;
  options->quiet = quiet;
}

  /**
   *  @fn char *dedup_options_get_reference_directory(dedup_options *options)
   *  @brief returns value of reference_directory from @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *
   *  @return string containing reference directory
   */

char *dedup_options_get_reference_directory(dedup_options *options)
{
  return options ? options->reference_directory : NULL;
}

  /**
   *  @fn void dedup_options_set_reference_directory(dedup_options *options,
   *                                                 char *reference_directory)
   *  @brief sets value of reference_directory in @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *  @param reference_directory - string containing new value for reference
   *                               directory
   *
   *  @par Returns
   *       Nothing
   */

void dedup_options_set_reference_directory(dedup_options *options, char *reference_directory)
{
  if (!options) return;
  if (options->reference_directory) free(options->reference_directory);
  options->reference_directory = NULL;
  if (reference_directory) options->reference_directory = strdup(reference_directory);
}

  /**
   *  @fn char *dedup_options_get_working_directory(dedup_options *options)
   *  @brief returns value of working_directory from @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *
   *  @return string containing reference directory
   */

char *dedup_options_get_working_directory(dedup_options *options)
{
  return options ? options->working_directory : NULL;
}

  /**
   *  @fn void dedup_options_set_working_directory(dedup_options *options,
   *                                               char *working_directory)
   *  @brief sets value of working_directory in @p options
   *
   *  @param options - pointer to existing @a dedup_options struct
   *  @param working_directory - string containing new value for reference
   *                             directory
   *
   *  @par Returns
   *       Nothing
   */

void dedup_options_set_working_directory(dedup_options *options, char *working_directory)
{
  if (!options) return;
  if (options->working_directory) free(options->working_directory);
  options->working_directory = NULL;
  if (working_directory) options->working_directory = strdup(working_directory);
}

  /**
   *  @fn dedup_hashes *dedup_hashes_new(void)
   *  @brief creates a new @a dedup_hashes struct
   *
   *  @par Parameters
   *       None.
   *
   *  @return pointer to new @a dedup_hashes struct
   */

dedup_hashes *dedup_hashes_new(void)
{
  dedup_hashes *root = NULL;

  root = malloc(sizeof(dedup_hashes));
  if (root) memset(root, 0, sizeof(dedup_hashes));
  root->hashes = avl_new();
  avl_set_new(root->hashes, (avl_new_node)dedup_node_new);
  avl_set_dup(root->hashes, (avl_dup_node)dedup_node_dup);
  avl_set_free(root->hashes, (avl_free_node)dedup_node_free);
  avl_set_cmp(root->hashes, (avl_cmp_node)dedup_node_cmp);

  return root;
}

  /**
   *  @fn dedup_hashes *dedup_hashes_build(char *root_path)
   *  @brief builds a new @a dedup_hashes struct
   *
   *  Each node in the AVL tree contains a unique HASH of the contents of file
   *
   *  The AVL tree contained in the struct allows for rapid lookup by HASH
   *
   *  @param root_path - string containing directory path
   *
   *  @return pointer to new @a dedup_hashes struct
   */

dedup_hashes *dedup_hashes_build(char *root_path)
{
  dedup_hashes *root = NULL;

  if (!root_path) goto exit;

  root = dedup_hashes_new();
  if (!root) goto exit;

  build_root(root, root_path);

exit:
  return root;
}

  /**
   *  @fn void dedup_hashes_free(dedup_hashes *tree)
   *  @brief frees all memory allocated to @p tree
   *
   *  @param tree - pointer to existing @a dedup_hashes struct
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_hashes_free(dedup_hashes *tree)
{
  if (!tree) return;
  if (tree->path) free(tree->path);
  if (tree->hashes) avl_free(tree->hashes);
  free(tree);
}

  /**
   *  @fn dedup_node *dedup_hashes_find_hash(dedup_hashes *tree, uint64_t hash)
   *  @brief locates node matching @p hash in @p tree
   *
   *  @param tree - pointer to existing @a dedup_hashes struct
   *  @param hash - HASH value to search find
   *
   *  @return pointer to @a dedup_node if found, NULL of not found
   */

dedup_node *dedup_hashes_find_hash(dedup_hashes *tree, uint64_t hash)
{
  dedup_node target;
  dedup_node *found = NULL;

  if (!tree) goto exit;

  target.hash = hash;
  found = (dedup_node *)avl_find(tree->hashes, (avl_node *)&target);

exit:
  return found;
}

  /**
   *  @fn dedup_node *dedup_node_new(void)
   *  @brief creates a new @a dedup_node struct
   *
   *  @par Parameters
   *       None.
   *
   *  @return pointer to new @a dedup_node struct
   */

dedup_node *dedup_node_new(void)
{
  dedup_node *node = NULL;

  if (!(node = malloc(sizeof(dedup_node)))) goto exit;

  memset(node, 0, sizeof(dedup_node));
  node->height = 1;

exit:
  return node;
}

  /**
   *  @fn dedup_node *dedup_node_dup(dedup_node *node)
   *  @brief makes a deep copy of @p node
   *
   *  @param node - pointer to @a dedup_node to copy
   *
   *  @return pointer to new @a dedup_node struct
   */

dedup_node *dedup_node_dup(dedup_node *node)
{
  dedup_node *new_node = NULL;

  if (!node) goto exit;

  new_node = dedup_node_new();
  if (!new_node) goto exit;

  memcpy(new_node, node, sizeof(dedup_node));
  if (node->pathname) new_node->pathname = strdup(node->pathname);

exit:
  return new_node;
}

  /**
   *  @fn void dedup_node_free(dedup_node *node)
   *  @brief frees all memory allocated to @p node
   *
   *  @param node - pointer to @a dedup_node
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_node_free(dedup_node *node)
{
  if (!node) return;

  if (node->pathname) free(node->pathname);
  free(node);
}

  /**
   *  @fn int dedup_node_cmp(dedup_node *a, dedup_node *b)
   *  @brief compares two @a dedup_node structs
   *
   *  @param a - pointer to @a dedup_node
   *  @param b - pointer to @a dedup_node
   *
   *  @return -1 if a<b, 0 if a==b, 1 if a>b
   */

int dedup_node_cmp(dedup_node *a, dedup_node *b)
{
  if (a->hash < b->hash) return -1;
  if (a->hash == b->hash) return 0;
  return 1;
}

  /**
   *  @fn char *dedup_node_get_pathname(dedup_node *node)
   *  @brief returns pathname value from @p node
   *
   *  @param node - pointer to @a dedup_node
   *
   *  @return string containing pathname value
   */

char *dedup_node_get_pathname(dedup_node *node)
{
  return node ? node->pathname : NULL;
}

  /**
   *  @fn void dedup_node_set_pathname(dedup_node *node, char *pathname)
   *  @brief sets pathname value in @p node
   *
   *  @param node - pointer to @a dedup_node
   *  @param pathname - string containing new pathname value
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_node_set_pathname(dedup_node *node, char *pathname)
{
  if (!node) return;
  if (node->pathname) free(node->pathname);
  node->pathname = NULL;
  if (pathname) node->pathname = strdup(pathname);
}

  /**
   *  @fn uint64_t dedup_node_get_hash(dedup_node *node)
   *  @brief returns HASH value from @p node
   *
   *  @param node - pointer to @a dedup_node
   *
   *  @return HASH value
   */

uint64_t dedup_node_get_hash(dedup_node *node) { return node ? node->hash : 0; }

  /**
   *  @fn void dedup_node_set_hash(dedup_node *node, uint64_t hash)
   *  @brief sets HASH value in @p node
   *
   *  @param node - pointer to @a dedup_node
   *  @param hash - new HASH value
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_node_set_hash(dedup_node *node, uint64_t hash)
{
  if (!node) return;
  node->hash = hash;
}

  /**
   *  @fn dedup_list *dedup_list_new(void)
   *  @brief creates a new @a dedup_list struct
   *
   *  @par Parameters
   *       None.
   *
   *  @return pointer to new @a dedup_list
   */

dedup_list *dedup_list_new(void)
{
  dedup_list *list = NULL;

  list = malloc(sizeof(dedup_list));
  if (list) memset(list, 0, sizeof(dedup_list));
  list->list = llist_new();
  llist_set_new(list->list, (llist_new_node)dedup_item_new);
  llist_set_dup(list->list, (llist_dup_node)dedup_item_dup);
  llist_set_free(list->list, (llist_free_node)dedup_item_free);
  llist_set_cmp(list->list, (llist_cmp_node)dedup_item_cmp);

  return list;
}

  /**
   *  @fn dedup_list *dedup_list_build(char *root_path)
   *  @brief builds a new @a dedup_list from @p root_path directory
   *
   *  @param root_path - string containing path to root of directory tree
   *
   *  @return pointer to new @a dedup_list
   */

dedup_list *dedup_list_build(char *root_path)
{
  dedup_list *list = NULL;

  if (!root_path) goto exit;

  list = dedup_list_new();
  if (!list) goto exit;

  build_list(list, root_path);

exit:
  return list;
}

  /**
   *  @fn void dedup_list_free(dedup_list *list)
   *  @brief frees all memory allocated to @p list
   *
   *  @param list - pointer to @a dedup_list struct
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_list_free(dedup_list *list)
{
  if (!list) return;
  if (list->path) free(list->path);
  if (list->list) llist_free(list->list);
  free(list);
}

  /**
   *  @fn dedup_item *dedup_list_head(dedup_list *list)
   *  @brief returns first item in @p list
   *
   *  @param list - pointer to @a dedup_list struct
   *
   *  @return pointer to head of list, NULL if none
   */

dedup_item *dedup_list_head(dedup_list *list)
{
  dedup_item *item = NULL;

  if (!list) goto exit;

  item = (dedup_item *) llist_head(list->list);

exit:
  return item;
}

  /**
   *  @fn dedup_item *dedup_list_tail(dedup_list *list)
   *  @brief returns first item in @p list
   *
   *  @param list - pointer to @a dedup_list struct
   *
   *  @return pointer to tail of list, NULL if none
   */

dedup_item *dedup_list_tail(dedup_list *list)
{
  dedup_item *item = NULL;

  if (!list) goto exit;

  item = (dedup_item *) llist_tail(list->list);

exit:
  return item;
}

  /**
   *  @fn dedup_item *dedup_list_current(dedup_list *list)
   *  @brief returns first item in @p list
   *
   *  @param list - pointer to @a dedup_list struct
   *
   *  @return pointer to current item in list, NULL if none
   */

dedup_item *dedup_list_current(dedup_list *list)
{
  dedup_item *item = NULL;

  if (!list) goto exit;

  item = (dedup_item *) llist_current(list->list);

exit:
  return item;
}

  /**
   *  @fn dedup_item *dedup_list_previous(dedup_list *list)
   *  @brief returns first item in @p list
   *
   *  @param list - pointer to @a dedup_list struct
   *
   *  @return pointer to previous item in list, NULL if none
   */

dedup_item *dedup_list_previous(dedup_list *list)
{
  dedup_item *item = NULL;

  if (!list) goto exit;

  item = (dedup_item *) llist_previous(list->list);

exit:
  return item;
}

  /**
   *  @fn dedup_item *dedup_list_next(dedup_list *list)
   *  @brief returns first item in @p list
   *
   *  @param list - pointer to @a dedup_list struct
   *
   *  @return pointer to next item in list, NULL if none
   */

dedup_item *dedup_list_next(dedup_list *list)
{
  dedup_item *item = NULL;

  if (!list) goto exit;

  item = (dedup_item *) llist_next(list->list);

exit:
  return item;
}

  /**
   *  @fn dedup_item *dedup_item_new(void)
   *  @brief creates a new @a dedup_item struct
   *
   *  @par Parameters
   *       None.
   *
   *  @return pointer to new @a dedup_item struct
   */

dedup_item *dedup_item_new(void)
{
  dedup_item *item = NULL;

  if (!(item = malloc(sizeof(dedup_item)))) goto exit;

  memset(item, 0, sizeof(dedup_item));

exit:
  return item;
}

  /**
   *  @fn dedup_item *dedup_item_dup(dedup_item *item)
   *  @brief makes a deep copy of @p item
   *
   *  @param item - pointer to @a dedup_item struct
   *
   *  @return pointer to new @a dedup_item struct
   */

dedup_item *dedup_item_dup(dedup_item *item)
{
  dedup_item *new_item = NULL;

  if (!item) goto exit;

  new_item = dedup_item_new();
  if (!new_item) goto exit;

  memcpy(new_item, item, sizeof(dedup_item));
  if (item->pathname) new_item->pathname = strdup(item->pathname);

exit:
  return new_item;
}

  /**
   *  @fn void dedup_item_free(dedup_item *item)
   *  @brief frees all memory allocated to @p item
   *
   *  @param item - pointer to @a dedup_item struct
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_item_free(dedup_item *item)
{
  if (!item) return;

  if (item->pathname) free(item->pathname);
  free(item);
}

  /**
   *  @fn int dedup_item_cmp(dedup_item *a, dedup_item *b)
   *  @brief compares two @a dedup_item structs
   *
   *  @param a - pointer to @a dedup_item struct
   *  @param b - pointer to @a dedup_item struct
   *
   *  @return -1 if a<b, 0 if a==b, 1 if a>b
   */

int dedup_item_cmp(dedup_item *a, dedup_item *b)
{
  int cv = 0;

  if (!a->pathname || !b->pathname) return 0;

  cv = strcmp(a->pathname, b->pathname);

  if (cv < 0) return -1;
  if (cv == 0) return 0;
  return 1;
}

  /**
   *  @fn char *dedup_item_get_pathname(dedup_item *item)
   *  @brief returns pathname value from @p item
   *
   *  @param item - pointer to @a dedup_item struct
   *
   *  @return string containing pathname, NULL if none
   */

char *dedup_item_get_pathname(dedup_item *item)
{
  return item ? item->pathname : NULL;
}

  /**
   *  @fn void dedup_item_set_pathname(dedup_item *item, char *pathname)
   *  @brief sets pathname value in @p item
   *
   *  @param item - pointer to @a dedup_item struct
   *  @param pathname - string containing new pathname value
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_item_set_pathname(dedup_item *item, char *pathname)
{
  if (!item) return;
  if (item->pathname) free(item->pathname);
  item->pathname = NULL;
  item->pathname = strdup(pathname);
}

  /**
   *  @fn uint64_t dedup_item_get_hash(dedup_item *item)
   *  @brief returns HASH value from @p item
   *
   *  @param item - pointer to @a dedup_item struct
   *
   *  @return HASH value
   */

uint64_t dedup_item_get_hash(dedup_item *item)
{
  return item ? item->hash : (uint64_t)0;
}

  /**
   *  @fn void dedup_item_set_hash(dedup_item *item, uint64_t hash)
   *  @brief sets HASH value in @p item
   *
   *  @param item - pointer to @a dedup_item struct
   *  @param hash - new HASH value
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_item_set_hash(dedup_item *item, uint64_t hash)
{
  if (!item) return;
  item->hash = hash;
}

  /**
   *  @fn uint64_t dedup_hash(void *data, unsigned int len)
   *  @brief creates 64 bit DJB hash value from @p data
   *
   *  @param data - pointer to generic memory location
   *  @param len - number of bytes in @p data
   *
   *  @return 64 bit DJB hash
   */

uint64_t dedup_hash(void *data, unsigned int len)
{
  uint64_t hash = 5381;
  uint8_t *m = (uint8_t *)data;
  int i;

  for (i = 0; i < len; i++)
    hash = ((hash << 5) + hash) + m[i];

  return hash;
}

  /**
   *  @fn char *dedup_hash_to_hex_str(uint64_t hash)
   *  @brief returns string representation of  @p hash
   *
   *  @param hash - 64 bit DJB hash value
   *
   *  @return 16 character long string containing hex value
   */

char *dedup_hash_to_hex_str(uint64_t hash)
{
  static char hex_str[17];
  uint8_t *array;
  int i;

  memset(hex_str, 0, 17);

  hash = htobe64(hash);
  array = (uint8_t *)&hash;

  for (i = 0; i < 8; i++) strcat(hex_str, uint8_to_hex(array[i]));

  return hex_str;
}

  /**
   *  @fn char *dedup_item_status(bool remove, char *pathname, int err)
   *  @brief builds a status string based on @p remove, @p pathname and @p err
   *
   *  @param remove - true of item is remove, false if just reported
   *  @param pathname - string containing pathname of item
   *  @param err - errno value set by unlink() function
   *
   *  @return formatted string for status report
   */

char *dedup_item_status(bool remove, char *pathname, int err)
{
  char *str = NULL;
  char *message = NULL;

  if (!pathname) goto exit;

  if (remove) message = "REMOVED";
  else message = "Would remove";

  if (remove && err)
  {
    str = strapp(str, "Removal of ");
    str = strapp(str, pathname);
    str = strapp(str, " failed: ");
    str = strapp(str, strerror(err));
  }
  else
  {
    str = strapp(str, message);
    str = strapp(str, " ");
    str = strapp(str, pathname);
  }

exit:
  return str;
}

  /**
   *  @fn void dedup_display_status(char *status, display_func display)
   *  @brief displays @p status string, using @p display if supplied
   *
   *  @param status - formatted string for status report
   *  @param display - pointer to @a display_func to call, if not NULL
   *
   *  @par Returns
   *       Nothing.
   */

void dedup_display_status(char *status, display_func display)
{
  if (!status) return;

  if (display) display(status);
  else printf("%s\n", status);
}

  /**
   *  @fn char *uint8_to_hex(uint8_t c)
   *  @brief converts value of @p c to 2 digit hex string
   *
   *  @param c - any byte value
   *
   *  @returns 2 digit hex string
   */

static char *uint8_to_hex(uint8_t c)
{
  static char hex[3];

  memset(hex, 0, 3);

  hex[0] = (c & 0xf0) >> 4;
  hex[0] = hex[0] < 10 ? hex[0] + '0' : hex[0] + 'A' - 10;
  
  hex[1] = c & 0xf;
  hex[1] = hex[1] < 10 ? hex[1] + '0' : hex[1] + 'A' - 10;
  
  return hex;
}

  /**
   *  @fn void build_root(dedup_hashes *root, char *root_path)
   *  @brief builds an AVL tree in @a dedup_hashes struct from contents
   *         of @p root_path
   *
   *  @param root - pointer to @a dedup_hashes struct
   *  @param root_path - string containing path of directory tree
   *
   *  @par Returns
   *       Nothing.
   */

static void build_root(dedup_hashes *root, char *root_path)
{
  DIR *dp = NULL;
  struct dirent *de;
  char *dir_path = NULL;
  char *pathname = NULL;
  struct stat st;
  size_t len;
  size_t read = 0;
  uint8_t *buf;
  FILE *f;
  dedup_node *node = NULL;

  if (!root || !root->hashes || !root_path) return;

  dp = opendir(root_path);
  if (!dp) return;

  while ((de = readdir(dp)))
  {
    if (!strcmp(de->d_name, ".")) continue;
    if (!strcmp(de->d_name, "..")) continue;

    switch (de->d_type)
    {
      case DT_DIR:
        dir_path = strapp(dir_path, root_path);
        dir_path = strapp(dir_path, "/");
        dir_path = strapp(dir_path, de->d_name);
        build_root(root, dir_path);
        free(dir_path);
        dir_path = NULL;

        break;

      case DT_REG:
        pathname = strapp(pathname, root_path);
        pathname = strapp(pathname, "/");
        pathname = strapp(pathname, de->d_name);

        memset(&st, 0, sizeof(struct stat));

        stat(pathname, &st);
        len = st.st_size;
        if (len <= 0) break;

        buf = malloc(len);
        if (!buf) goto cleanup;
        memset(buf, 0, len);

        f = fopen(pathname, "r");
        read = fread(buf, 1, len, f);
        fclose(f);
        if (read != len) goto cleanup;

        node = dedup_node_new();
        node->pathname = strdup(pathname);
        node->hash = dedup_hash(buf, len);
        avl_insert(root->hashes, (avl_node *)node);

        break;

      default: break;
    }

cleanup:
    if (buf) free(buf);
    if (pathname) free(pathname);
    buf = NULL;
    pathname = NULL;
  }

  closedir(dp);

  if (dir_path) free(dir_path);
}

  /**
   *  @fn void build_list(dedup_list *list, char *root_path)
   *  @brief builds an llist in @a dedup_list struct from contents
   *         of @p root_path
   *
   *  @param list - pointer to @a dedup_list struct
   *  @param root_path - string containing path of directory tree
   *
   *  @par Returns
   *       Nothing.
   */

static void build_list(dedup_list *list, char *root_path)
{
  DIR *dp = NULL;
  struct dirent *de;
  char *dir_path = NULL;
  char *pathname = NULL;
  struct stat st;
  size_t len;
  size_t read = 0;
  uint8_t *buf;
  FILE *f;
  dedup_item *item = NULL;

  if (!list || !list->list || !root_path) return;

  dp = opendir(root_path);
  if (!dp) return;

  while ((de = readdir(dp)))
  {
    if (!strcmp(de->d_name, ".")) continue;
    if (!strcmp(de->d_name, "..")) continue;

    switch (de->d_type)
    {
      case DT_DIR:
        dir_path = strapp(dir_path, root_path);
        dir_path = strapp(dir_path, "/");
        dir_path = strapp(dir_path, de->d_name);
        build_list(list, dir_path);
        free(dir_path);
        dir_path = NULL;

        break;

      case DT_REG:
        pathname = strapp(pathname, root_path);
        pathname = strapp(pathname, "/");
        pathname = strapp(pathname, de->d_name);

        memset(&st, 0, sizeof(struct stat));

        stat(pathname, &st);
        len = st.st_size;
        if (len < 0) break;

        if (len)
        {
          buf = malloc(len);
          if (!buf) goto cleanup;
          memset(buf, 0, len);

          f = fopen(pathname, "r");
          read = fread(buf, 1, len, f);
          fclose(f);
          if (read != len) break;
        }

        item = dedup_item_new();
        item->pathname = pathname;
        item->hash = len ? dedup_hash(buf, len) : 0;
        llist_add(list->list, llist_position_tail, NULL, (llist_node *)item);

        break;

      default: break;
    }

cleanup:
    if (buf) free(buf);
    if (pathname) free(pathname);
    buf = NULL;
    pathname = NULL;
  }

  closedir(dp);

  if (dir_path) free(dir_path);
}

