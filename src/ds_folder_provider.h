/*  Copyright 2021-2022 MarcosHCK
 *  This file is part of deusexmakina2.
 *
 *  deusexmakina2 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  deusexmakina2 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with deusexmakina2.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __DS_FOLDER_PROVIDER_INCLUDED__
#define __DS_FOLDER_PROVIDER_INCLUDED__ 1
#include <ds_export.h>
#include <gio/gio.h>

/**
 * DS_FOLDER_PROVIDER_ERROR:
 *
 * Error domain for DS_FOLDER_PROVIDER. Errors in this domain will be from the #DsFolderProviderError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_FOLDER_PROVIDER_ERROR (ds_folder_provider_error_quark())

/**
 * DsFolderProviderError:
 * @DS_FOLDER_PROVIDER_ERROR_FAILED: generic error condition.
 * @DS_FOLDER_PROVIDER_ERROR_INVALID: invalid default provider.
 *
 * Error code returned by DsCacheProvider API.
 * Note that %DS_FOLDER_PROVIDER_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum
{
  DS_FOLDER_PROVIDER_ERROR_FAILED,
  DS_FOLDER_PROVIDER_ERROR_INVALID,
} DsFolderProviderError;

#define DS_TYPE_FOLDER_PROVIDER             (ds_folder_provider_get_type ())
#define DS_FOLDER_PROVIDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_FOLDER_PROVIDER, DsFolderProvider))
#define DS_FOLDER_PROVIDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_FOLDER_PROVIDER, DsFolderProviderClass))
#define DS_IS_FOLDER_PROVIDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_FOLDER_PROVIDER))
#define DS_IS_FOLDER_PROVIDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_FOLDER_PROVIDER))
#define DS_FOLDER_PROVIDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_FOLDER_PROVIDER, DsFolderProviderClass))

typedef struct _DsFolderProvider        DsFolderProvider;
typedef struct _DsFolderProviderClass   DsFolderProviderClass;
typedef struct _DsFolderProviderPrivate DsFolderProviderPrivate;

#define DS_TYPE_DATA_PROVIDER             (ds_data_provider_get_type ())
#define DS_DATA_PROVIDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_DATA_PROVIDER, DsDataProvider))
#define DS_DATA_PROVIDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_DATA_PROVIDER, DsDataProviderClass))
#define DS_IS_DATA_PROVIDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_DATA_PROVIDER))
#define DS_IS_DATA_PROVIDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_DATA_PROVIDER))
#define DS_DATA_PROVIDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_DATA_PROVIDER, DsDataProviderClass))

typedef struct _DsDataProvider          DsDataProvider;
typedef struct _DsDataProviderClass     DsDataProviderClass;
typedef struct _DsDataProviderPrivate   DsDataProviderPrivate;

#define DS_TYPE_CACHE_PROVIDER            (ds_cache_provider_get_type ())
#define DS_CACHE_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_CACHE_PROVIDER, DsCacheProvider))
#define DS_CACHE_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_CACHE_PROVIDER, DsCacheProviderClass))
#define DS_IS_CACHE_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_CACHE_PROVIDER))
#define DS_IS_CACHE_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_CACHE_PROVIDER))
#define DS_CACHE_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_CACHE_PROVIDER, DsCacheProviderClass))

typedef struct _DsCacheProvider         DsCacheProvider;
typedef struct _DsCacheProviderClass    DsCacheProviderClass;
typedef struct _DsCacheProviderPrivate  DsCacheProviderPrivate;

#if __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Types
 *
 */

DEUSEXMAKINA2_API
GQuark
ds_folder_provider_error_quark();
DEUSEXMAKINA2_API
GType
ds_folder_provider_get_type();
DEUSEXMAKINA2_API
GType
ds_data_provider_get_type();
DEUSEXMAKINA2_API
GType
ds_cache_provider_get_type();

/*
 * Structs
 *
 */

struct _DsFolderProvider
{
  GObject parent_instance;
  DsFolderProviderPrivate * priv;
  GFile* basedir;
};

struct _DsFolderProviderClass
{
  GObjectClass parent_class;
  GFile* (*pick_basedir) (DsFolderProvider* self, GCancellable* cancellable, GError** error);
};

struct _DsDataProvider
{
  GObject parent_instance;
  DsDataProviderPrivate * priv;
};

struct _DsDataProviderClass
{
  GObjectClass parent_class;
};

struct _DsCacheProvider
{
  GObject parent_instance;
  DsCacheProviderPrivate * priv;
};

struct _DsCacheProviderClass
{
  GObjectClass parent_class;
};

/*
 * Functions
 *
 */

/**
 * ds_folder_provider_child:
 * @provider: an #DsFolderProvider instance.
 * @name: child name.
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Request an specified child folder to be available (if it doesn't exists
 * is created).
 * 
 * Returns: (transfer full): requested child.
 */
DEUSEXMAKINA2_API
GFile*
ds_folder_provider_child (DsFolderProvider   *provider,
                          const gchar        *name,
                          GCancellable       *cancellable,
                          GError            **error);

/**
 * ds_data_provider_new:
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Creates a new instance of #DsDataProvider on
 * default data folder (see source for a step by
 * step description of how it is selected).
 * 
 * Returns: (transfer full): see description.
 */
DEUSEXMAKINA2_API
DsDataProvider*
ds_data_provider_new (GCancellable* cancellable,
                      GError** error);

/**
 * ds_data_provider_get_default:
 *
 * Retrieves default per-process instance of #DsDataProvider,
 * if it was previously created.
 * 
 * Returns: (transfer none) (nullable): default per-process provider.
 */
DEUSEXMAKINA2_API
DsDataProvider*
ds_data_provider_get_default (void);

/**
 * ds_cache_provider_new:
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Creates a new instance of #DsCacheProvider on
 * default cache folder (see source for a step by
 * step description of how it is selected).
 * 
 * Returns: (transfer full): see description.
 */
DEUSEXMAKINA2_API
DsCacheProvider*
ds_cache_provider_new (GCancellable   *cancellable,
                       GError        **error);

/**
 * ds_cache_provider_get_default:
 *
 * Retrieves default per-process instance of #DsCacheProvider,
 * if it was previously created.
 * 
 * Returns: (transfer none) (nullable): default per-process provider.
 */
DEUSEXMAKINA2_API
DsCacheProvider*
ds_cache_provider_get_default (void);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_FOLDER_PROVIDER_INCLUDED__
