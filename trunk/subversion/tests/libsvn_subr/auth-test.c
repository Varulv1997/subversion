/*
 * auth-test.c -- test the auth functions
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

#include "svn_auth.h"
#include "svn_dirent_uri.h"
#include "svn_private_config.h"

#include "../svn_test.h"
#include "private/svn_auth_private.h"

static svn_error_t *
test_platform_specific_auth_providers(apr_pool_t *pool)
{
  apr_array_header_t *providers;
  svn_auth_provider_object_t *provider;
  int number_of_providers = 0;

  /* Test non-available auth provider */
  SVN_ERR(svn_auth_get_platform_specific_provider(&provider, "fake", "fake",
                                                  pool));

  if (provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('fake', 'fake') should " \
       "return NULL");

  /* Make sure you get appropriate number of providers when retrieving
     all auth providers */
  SVN_ERR(svn_auth_get_platform_specific_client_providers(&providers, NULL,
                                                          pool));

#ifdef SVN_HAVE_GNOME_KEYRING
  number_of_providers += 2;
#endif
#ifdef SVN_HAVE_KWALLET
  number_of_providers += 2;
#endif
#ifdef SVN_HAVE_GPG_AGENT
  number_of_providers += 1;
#endif
#ifdef SVN_HAVE_KEYCHAIN_SERVICES
  number_of_providers += 2;
#endif
#if defined(WIN32) && !defined(__MINGW32__)
  number_of_providers += 4;
#endif
  if (providers->nelts != number_of_providers)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_client_providers should return " \
       "an array of %d providers, but returned %d providers",
       number_of_providers, providers->nelts);

  /* Test Keychain auth providers */
#ifdef SVN_HAVE_KEYCHAIN_SERVICES
  svn_auth_get_platform_specific_provider(&provider, "keychain",
                                          "simple", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('keychain', 'simple') "
       "should not return NULL");

  svn_auth_get_platform_specific_provider(&provider, "keychain",
                                          "ssl_client_cert_pw", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('keychain', " \
       "'ssl_client_cert_pw') should not return NULL");

  /* Make sure you do not get a Windows auth provider */
  svn_auth_get_platform_specific_provider(&provider, "windows",
                                          "simple", pool);

  if (provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('windows', 'simple') should " \
       "return NULL");
#endif

  /* Test Windows auth providers */
#if defined(WIN32) && !defined(__MINGW32__)
  svn_auth_get_platform_specific_provider(&provider, "windows",
                                          "simple", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('windows', 'simple') "
       "should not return NULL");


  svn_auth_get_platform_specific_provider(&provider, "windows",
                                          "ssl_client_cert_pw", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('windows', "
       "'ssl_client_cert_pw') should not return NULL");

  svn_auth_get_platform_specific_provider(&provider, "windows",
                                          "ssl_server_trust", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('windows', "
       "'ssl_server_trust') should not return NULL");

  /* Make sure you do not get a Keychain auth provider */
  svn_auth_get_platform_specific_provider(&provider, "keychain",
                                          "simple", pool);

  if (provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('keychain', 'simple') should " \
       "return NULL");
#endif

  /* Test GNOME Keyring auth providers */
#ifdef SVN_HAVE_GNOME_KEYRING
  SVN_ERR(svn_auth_get_platform_specific_provider(&provider, "gnome_keyring",
                                                  "simple", pool));

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('gnome_keyring', 'simple') "
       "should not return NULL");

  SVN_ERR(svn_auth_get_platform_specific_provider(&provider, "gnome_keyring",
                                                  "ssl_client_cert_pw", pool));

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('gnome_keyring', " \
       "'ssl_client_cert_pw') should not return NULL");

  /* Make sure you do not get a Windows auth provider */
  SVN_ERR(svn_auth_get_platform_specific_provider(&provider, "windows",
                                                  "simple", pool));

  if (provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('windows', 'simple') should " \
       "return NULL");
#endif

  /* Test KWallet auth providers */
#ifdef SVN_HAVE_KWALLET
  svn_auth_get_platform_specific_provider(&provider, "kwallet",
                                          "simple", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('kwallet', 'simple') "
       "should not return NULL");

  svn_auth_get_platform_specific_provider(&provider, "kwallet",
                                          "ssl_client_cert_pw", pool);

  if (!provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('kwallet', " \
       "'ssl_client_cert_pw') should not return NULL");

  /* Make sure you do not get a Windows auth provider */
  svn_auth_get_platform_specific_provider(&provider, "windows",
                                          "simple", pool);

  if (provider)
    return svn_error_createf
      (SVN_ERR_TEST_FAILED, NULL,
       "svn_auth_get_platform_specific_provider('windows', 'simple') should " \
       "return NULL");
#endif

  return SVN_NO_ERROR;
}

/* Helper for test_auth_clear(). Implements svn_config_auth_walk_func_t */
static svn_error_t *
cleanup_callback(svn_boolean_t *delete_cred,
                 void *walk_baton,
                 const char *cred_kind,
                 const char *realmstring,
                 apr_hash_t *cred_hash,
                 apr_pool_t *scratch_pool)
{
  svn_auth_baton_t *b = walk_baton;

  SVN_TEST_ASSERT(strcmp(cred_kind, SVN_AUTH_CRED_SIMPLE) == 0);
  SVN_TEST_ASSERT(strcmp(realmstring, "<http://my.host> My realm") == 0);

  SVN_ERR(svn_auth_forget_credentials(b, cred_kind, realmstring, scratch_pool));

  *delete_cred = TRUE;

  return SVN_NO_ERROR;
}

static svn_error_t *
test_auth_clear(apr_pool_t *pool)
{
  const char *auth_dir;
  svn_auth_provider_object_t *provider;
  svn_auth_baton_t *baton;
  apr_array_header_t *providers;
  void *credentials;
  svn_auth_cred_simple_t *creds;
  svn_auth_iterstate_t *state;

  SVN_ERR(svn_dirent_get_absolute(&auth_dir, "", pool));
  auth_dir = svn_dirent_join(auth_dir, "auth-clear", pool);

  svn_test_add_dir_cleanup(auth_dir);

  SVN_ERR(svn_io_remove_dir2(auth_dir, TRUE, NULL, NULL, pool));
  SVN_ERR(svn_io_dir_make(auth_dir, APR_OS_DEFAULT, pool));

  svn_auth_get_simple_provider2(&provider, NULL, NULL, pool);

  providers = apr_array_make(pool, 1, sizeof(svn_auth_provider_object_t *));
  APR_ARRAY_PUSH(providers, svn_auth_provider_object_t *) = provider;

  svn_auth_open(&baton, providers, pool);

  svn_auth_set_parameter(baton, SVN_AUTH_PARAM_DEFAULT_USERNAME, "jrandom");
  svn_auth_set_parameter(baton, SVN_AUTH_PARAM_DEFAULT_PASSWORD, "rayjandom");
  svn_auth_set_parameter(baton, SVN_AUTH_PARAM_CONFIG_DIR, auth_dir);

  /* Create the auth subdirs. Without these we can't store passwords */
  SVN_ERR(svn_config_ensure(auth_dir, pool));

  /* Obtain the default credentials just passed */
  SVN_ERR(svn_auth_first_credentials(&credentials,
                                     &state,
                                     SVN_AUTH_CRED_SIMPLE,
                                     "<http://my.host> My realm",
                                     baton,
                                     pool));

  creds = credentials;
  SVN_TEST_ASSERT(strcmp(creds->username, "jrandom") == 0);
  SVN_TEST_ASSERT(creds->may_save);

  /* And tell that they are ok and can be saved */
  SVN_ERR(svn_auth_save_credentials(state, pool));

  /* Ok, and now we try to remove the credentials */
  svn_auth_set_parameter(baton, SVN_AUTH_PARAM_DEFAULT_USERNAME, NULL);
  svn_auth_set_parameter(baton, SVN_AUTH_PARAM_DEFAULT_PASSWORD, NULL);

  /* Are they still in the baton? */
  SVN_ERR(svn_auth_first_credentials(&credentials,
                                     &state,
                                     SVN_AUTH_CRED_SIMPLE,
                                     "<http://my.host> My realm",
                                     baton,
                                     pool));

  SVN_TEST_ASSERT(credentials);
  creds = credentials;
  SVN_TEST_ASSERT(strcmp(creds->username, "jrandom") == 0);
  SVN_TEST_ASSERT(creds->may_save);

  /* Use our walker function to delete credentials (and forget them
     from the auth baton). */
  SVN_ERR(svn_config_walk_auth_data(auth_dir, cleanup_callback, baton, pool));

  /* Finally, they should be gone! */
  SVN_ERR(svn_auth_first_credentials(&credentials,
                                     &state,
                                     SVN_AUTH_CRED_SIMPLE,
                                     "<http://my.host> My realm",
                                     baton,
                                     pool));

  SVN_TEST_ASSERT(! credentials);

  return SVN_NO_ERROR;
}


/* The test table.  */

struct svn_test_descriptor_t test_funcs[] =
  {
    SVN_TEST_NULL,
    SVN_TEST_PASS2(test_platform_specific_auth_providers,
                   "test retrieving platform-specific auth providers"),
    SVN_TEST_PASS2(test_auth_clear,
                   "test svn_auth_clear()"),
    SVN_TEST_NULL
  };
