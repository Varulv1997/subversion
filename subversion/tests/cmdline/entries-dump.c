/*
 * entries-dump.c :  dump pre-1.6 svn_wc_* output for python
 *
 * ====================================================================
 *    Licensed to the Apache Software Foundation (ASF) under one
 *    or more contributor license agreements.  See the NOTICE file
 *    distributed with this work for additional information
 *    regarding copyright ownership.  The ASF licenses this file
 *    to you under the Apache License, Version 2.0 (the
 *    "License"); you may not use this file except in compliance
 *    with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing,
 *    software distributed under the License is distributed on an
 *    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *    KIND, either express or implied.  See the License for the
 *    specific language governing permissions and limitations
 *    under the License.
 * ====================================================================
 */

#include <stdlib.h>
#include <stdio.h>

#include <apr_pools.h>
#include <apr_general.h>

#include "svn_types.h"
#include "svn_pools.h"
#include "svn_wc.h"
#include "svn_dirent_uri.h"

#include "private/svn_wc_private.h"

static void
str_value(const char *name, const char *value)
{
  if (value == NULL)
    printf("e.%s = None\n", name);
  else
    printf("e.%s = '%s'\n", name, value);
}


static void
int_value(const char *name, long int value)
{
  printf("e.%s = %ld\n", name, value);
}


static void
bool_value(const char *name, svn_boolean_t value)
{
  if (value)
    printf("e.%s = True\n", name);
  else
    printf("e.%s = False\n", name);
}

static svn_error_t *
entries_dump(const char *dir_path, apr_pool_t *pool)
{
  svn_wc_adm_access_t *adm_access;
  apr_hash_t *entries;
  apr_hash_index_t *hi;
  svn_boolean_t locked;

  SVN_ERR(svn_wc_locked(&locked, dir_path, pool));
  SVN_ERR(svn_wc_adm_open3(&adm_access, NULL, dir_path, FALSE, 0,
                           NULL, NULL, pool));
  SVN_ERR(svn_wc_entries_read(&entries, adm_access, TRUE, pool));

  for (hi = apr_hash_first(pool, entries); hi; hi = apr_hash_next(hi))
    {
      const void *key;
      void *value;
      const svn_wc_entry_t *entry;

      apr_hash_this(hi, &key, NULL, &value);
      entry = value;

      SVN_ERR_ASSERT(strcmp(key, entry->name) == 0);

      printf("e = Entry()\n");
      str_value("name", entry->name);
      int_value("revision", entry->revision);
      str_value("url", entry->url);
      str_value("repos", entry->repos);
      str_value("uuid", entry->uuid);
      int_value("kind", entry->kind);
      int_value("schedule", entry->schedule);
      bool_value("copied", entry->copied);
      bool_value("deleted", entry->deleted);
      bool_value("absent", entry->absent);
      bool_value("incomplete", entry->incomplete);
      str_value("copyfrom_url", entry->copyfrom_url);
      int_value("copyfrom_rev", entry->copyfrom_rev);
      str_value("conflict_old", entry->conflict_old);
      str_value("conflict_new", entry->conflict_new);
      str_value("conflict_wrk", entry->conflict_wrk);
      str_value("prejfile", entry->prejfile);
      /* skip: text_time */
      /* skip: prop_time */
      /* skip: checksum */
      int_value("cmt_rev", entry->cmt_rev);
      /* skip: cmt_date */
      str_value("cmt_author", entry->cmt_author);
      str_value("lock_token", entry->lock_token);
      str_value("lock_owner", entry->lock_owner);
      str_value("lock_comment", entry->lock_comment);
      /* skip: lock_creation_date */
      /* skip: has_props */
      /* skip: has_prop_mods */
      /* skip: cachable_props */
      /* skip: present_props */
      str_value("changelist", entry->changelist);
      /* skip: working_size */
      /* skip: keep_local */
      int_value("depth", entry->depth);
      /* skip: tree_conflict_data */
      /* skip: file_external_path */
      /* skip: file_external_peg_rev */
      /* skip: file_external_rev */
      bool_value("locked", locked && *entry->name == '\0');
      printf("entries['%s'] = e\n", (const char *)key);
    }

  return svn_wc_adm_close2(adm_access, pool);
}


/* baton for print_dir */
struct directory_walk_baton
{
  svn_wc_context_t *wc_ctx;
  const char *root_abspath;
  const char *prefix_path;
};

/* svn_wc__node_found_func_t implementation for directory_dump */
static svn_error_t *
print_dir(const char *local_abspath,
          svn_node_kind_t kind,
          void *walk_baton,
          apr_pool_t *scratch_pool)
{
  struct directory_walk_baton *bt = walk_baton;

  if (kind != svn_node_dir)
    return SVN_NO_ERROR;

  printf("%s\n",
         svn_dirent_local_style(
                   svn_dirent_join(bt->prefix_path,
                                   svn_dirent_skip_ancestor(bt->root_abspath,
                                                            local_abspath),
                                   scratch_pool),
                   scratch_pool));

  return SVN_NO_ERROR;
}

/* Print all not-hidden subdirectories in the working copy, starting by path */
static svn_error_t *
directory_dump(const char *path,
               apr_pool_t *scratch_pool)
{
  struct directory_walk_baton bt;

  SVN_ERR(svn_wc_context_create(&bt.wc_ctx, NULL, scratch_pool, scratch_pool));
  SVN_ERR(svn_dirent_get_absolute(&bt.root_abspath, path, scratch_pool));

  bt.prefix_path = path;

  SVN_ERR(svn_wc__node_walk_children(bt.wc_ctx, bt.root_abspath, FALSE,
                                     print_dir, &bt, svn_depth_infinity,
                                     NULL, NULL, scratch_pool));

  return svn_error_return(svn_wc_context_destroy(bt.wc_ctx));
}

int
main(int argc, const char *argv[])
{
  apr_pool_t *pool;
  int exit_code = EXIT_SUCCESS;
  svn_error_t *err;
  const char *path;
  const char *cmd;

  if (argc < 2 || argc > 4)
    {
      fprintf(stderr, "USAGE: entries-dump [--entries|--subdirs] DIR_PATH\n");
      exit(1);
    }

  if (apr_initialize() != APR_SUCCESS)
    {
      fprintf(stderr, "apr_initialize() failed.\n");
      exit(1);
    }

  /* set up the global pool */
  pool = svn_pool_create(NULL);

  path = svn_dirent_internal_style(argv[argc-1], pool);

  if (argc > 2)
    cmd = argv[1];
  else
    cmd = NULL;

  if (!cmd || !strcmp(cmd, "--entries"))
    err = entries_dump(path, pool);
  else if (!strcmp(cmd, "--subdirs"))
    err = directory_dump(path, pool);
  else
    err = svn_error_createf(SVN_ERR_INCORRECT_PARAMS, NULL,
                            "Invalid command '%s'",
                            cmd);
  if (err)
    {
      svn_handle_error2(err, stderr, FALSE, "entries-dump: ");
      svn_error_clear(err);
      exit_code = EXIT_FAILURE;
    }

  /* Clean up, and get outta here */
  svn_pool_destroy(pool);
  apr_terminate();

  return exit_code;
}
