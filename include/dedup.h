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
 *  @file dedup.h
 *  @brief cli command and library to dedup all indentical files from a
 *         directory compared to a different reference directory
 */

#ifndef DEDUP_H
#define DEDUP_H

#include <llist.h>
#include <avl.h>

  /**
   *  @typedef void (*display_func)(char *status)
   *  @brief creates function fingerprint for user specific item status display
   */

typedef void (*display_func)(char *status);

  /**
   *  @typedef struct dedup_options dedup_options;
   *  @brief creates a type for @a dedup_options struct
   */

typedef struct dedup_options dedup_options;

  /**
   *  @struct dedup_options
   *  @brief options used during dedup process
   */

struct dedup_options
{
  bool remove_files;          /**<  flag, actually remove files when true    */
  bool force_removal;         /**<  flag, don't issue warning to user        */
  bool quiet;                 /**<  flag, don't display options or progress  */
  char *reference_directory;  /**<  directory to check against               */
  char *working_directory;    /**<  directory where dups are removed         */
};

  /**
   *  @typedef struct dedup_node dedup_node;
   *  @brief creates a type for @a dedup_node struct
   */

typedef struct dedup_node dedup_node;

  /**
   *  @struct dedup_node
   *  @brief node stored in @a dedup_hashes.hashes AVL tree
   */

struct dedup_node
{
  avl_node *left;   /**<  points to left (lesser) node    */
  avl_node *right;  /**<  points to right (greater) node  */
  int height;       /**<  current height of node          */
  char *pathname;   /**<  relative path and name of file   */
  uint64_t hash;    /**<  hash of contents of @a pathname  */
};

  /**
   *  @typedef struct dedup_hashes dedup_hashes;
   *  @brief creates a type for @a dedup_hashes struct
   */

typedef struct dedup_hashes dedup_hashes;

  /**
   *  @struct dedup_hashes
   *  @brief tracks unique file hashes
   */

struct dedup_hashes
{
  char *path;   /**<  path to root of file tree                              */
  avl *hashes;  /**<  avl tree containing hash values for each file in tree  */
};

  /**
   *  @typedef struct dedup_item dedup_item;
   *  @brief creates a type for @a dedup_item struct
   */

typedef struct dedup_item dedup_item;

  /**
   *  @struct dedup_item
   *  @brief node stored in @a dedup_list.hashes AVL tree
   */

struct dedup_item
{
  llist_node *previous;  /**<  points to previous node in the list    */
  llist_node *next;      /**<  points to next node in the list  */
  llist_node *current;   /**<  points to current node in the list  */
  char *pathname;        /**<  relative path and name of file   */
  uint64_t hash;         /**<  hash of contents of @a pathname  */
};

  /**
   *  @typedef struct dedup_list dedup_list;
   *  @brief creates a type for @a dedup_list struct
   */

typedef struct dedup_list dedup_list;

  /**
   *  @struct dedup_list
   *  @brief tracks unique file hashes
   */

struct dedup_list
{
  char *path;   /**<  path to root of file tree                                     */
  llist *list;  /**<  llist containing paths and hash values for each file in tree  */
};

  /*
   *  Main processing function
   */

void dedup(dedup_options *options);

  /*
   *  Functions to manage dedup_options struct
   */

dedup_options *dedup_options_new(void);
void dedup_options_free(dedup_options *options);
bool dedup_options_get_remove_files(dedup_options *options);
void dedup_options_set_remove_files(dedup_options *options, bool remove_files);
bool dedup_options_get_force_removal(dedup_options *options);
void dedup_options_set_force_removal(dedup_options *options, bool force_removal);
bool dedup_options_get_quiet(dedup_options *options);
void dedup_options_set_quiet(dedup_options *options, bool quiet);
char *dedup_options_get_reference_directory(dedup_options *options);
void dedup_options_set_reference_directory(dedup_options *options, char *reference_directory);
char *dedup_options_get_working_directory(dedup_options *options);
void dedup_options_set_working_directory(dedup_options *options, char *working_directory);

  /*
   *  Functions to manage dedup_hashes struct
   */

dedup_hashes *dedup_hashes_new(void);
dedup_hashes *dedup_hashes_build(char *root_path);
void dedup_hashes_free(dedup_hashes *tree);
dedup_node *dedup_hashes_find_hash(dedup_hashes *tree, uint64_t hash);

  /*
   *  Functions to manage dedup_node struct, used by dedup_hashes
   */

dedup_node *dedup_node_new(void);
dedup_node *dedup_node_dup(dedup_node *node);
void dedup_node_free(dedup_node *node);
int dedup_node_cmp(dedup_node *a, dedup_node *b);
char *dedup_node_get_pathname(dedup_node *node);
void dedup_node_set_pathname(dedup_node *node, char *pathname);
uint64_t dedup_node_get_hash(dedup_node *node);
void dedup_node_set_hash(dedup_node *node, uint64_t hash);

  /*
   *  Functions to manage dedup_list struct
   */

dedup_list *dedup_list_new(void);
dedup_list *dedup_list_build(char *root_path);
void dedup_list_free(dedup_list *list);
dedup_item *dedup_list_head(dedup_list *list);
dedup_item *dedup_list_tail(dedup_list *list);
dedup_item *dedup_list_current(dedup_list *list);
dedup_item *dedup_list_previous(dedup_list *list);
dedup_item *dedup_list_next(dedup_list *list);

  /*
   *  Functions to manage dedup_item struct, used by dedup_list
   */

dedup_item *dedup_item_new(void);
dedup_item *dedup_item_dup(dedup_item *item);
void dedup_item_free(dedup_item *item);
int dedup_item_cmp(dedup_item *a, dedup_item *b);
char *dedup_item_get_pathname(dedup_item *item);
void dedup_item_set_pathname(dedup_item *item, char *pathname);
uint64_t dedup_item_get_hash(dedup_item *item);
void dedup_item_set_hash(dedup_item *item, uint64_t hash);

  /*
   *  Utility functions
   */

uint64_t dedup_hash(void *data, unsigned int len);
char *dedup_hash_to_hex_str(uint64_t hash);
char *dedup_item_status(bool remove, char *pathname, int err);
void dedup_display_status(char *status, display_func display);

#endif //DEDUP_H
