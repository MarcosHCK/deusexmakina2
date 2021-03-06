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
#ifndef __DS_MODEL_INCLUDED__
#define __DS_MODEL_INCLUDED__ 1
#include <ds_export.h>
#include <ds_pencil.h>
#include <gio/gio.h>

/**
 * DS_MODEL_ERROR:
 *
 * Error domain for DS_MODEL. Errors in this domain will be from the #DsModelError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_MODEL_ERROR (ds_model_error_quark())

/**
 * DsModelError:
 * @DS_MODEL_ERROR_FAILED: generic error condition.
 * @DS_MODEL_ERROR_NO_INPUT: no input supplied at initialization time.
 * @DS_MODEL_ERROR_INCOMPLETE_IMPORT: incomplete or totally failed import process.
 * @DS_MODEL_ERROR_TEXTURE_LOAD: error loading texture.
 *
 * Error code returned by DsModel API.
 * Note that %DS_MODEL_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_MODEL_ERROR_FAILED,
  DS_MODEL_ERROR_NO_INPUT,
  DS_MODEL_ERROR_INCOMPLETE_IMPORT,
  DS_MODEL_ERROR_TEXTURE_LOAD,
} DsModelError;

#define DS_TYPE_MODEL             (ds_model_get_type ())
#define DS_MODEL(object)          (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_MODEL, DsModel))
#define DS_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_MODEL, DsModelClass))
#define DS_IS_MODEL(object)       (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_MODEL))
#define DS_IS_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_MODEL))
#define DS_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_MODEL, DsModelClass))

typedef struct _DsModel             DsModel;
typedef struct _DsModelPrivate      DsModelPrivate;
typedef struct _DsModelClass        DsModelClass;
typedef         guint               DsModelIndex;

G_BEGIN_DECLS

/**
 * DsModel:
 * @parent_instance: parent instance.
 * @bos: OpenGL buffer object names array.
 * @vbo: OpenGL vertex buffer object name (where vertices data
 * are stored).
 * @ibo: OpenGL index buffer object name (where indices data
 * are stored).
 *
 */
struct _DsModel
{
  GObject parent_instance;

  /*<private>*/
  DsModelPrivate* priv;

  /*<public>*/
  union
  {
    guint bos[2];
    struct
    {
      guint vbo;
      guint ibo;
    };
  };
};

/**
 * DsModelClass:
 * @parent_class: parent class.
 * @vao: OpenGL vertex array object name.
 * @compile: forwards #DsRenderable::compile virtual function implementation,
 * since #DsModel already implements it by itself.
 *
 */
struct _DsModelClass
{
  GObjectClass parent_class;

  /*<public>*/
  gboolean (*compile) (DsModel* model, DsRenderState* state, GCancellable* cancellable, GError** error);
};

DEUSEXMAKINA2_API
GQuark
ds_model_error_quark();
DEUSEXMAKINA2_API
GType
ds_model_get_type();

/*
 * Model types
 *
 */

DEUSEXMAKINA2_API
GType
ds_model_single_get_type();
DEUSEXMAKINA2_API
DsModel*
ds_model_single_new(GFile* source, const gchar* name, GCancellable* cancellable, GError** error);

G_END_DECLS

#endif // __DS_MODEL_INCLUDED__
