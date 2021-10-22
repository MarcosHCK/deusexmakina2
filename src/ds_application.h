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
#ifndef __DS_APPLICATION_INCLUDED__
#define __DS_APPLICATION_INCLUDED__ 1
#include <ds_events.h>
#include <ds_export.h>
#include <ds_pipeline.h>
#include <ds_renderer.h>
#include <gio/gio.h>

/**
 * DS_APPLICATION_ERROR:
 *
 * Error domain for DS_APPLICATION. Errors in this domain will be from the #DsApplicationError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_APPLICATION_ERROR  (ds_application_error_quark())

/**
 * DsApplicationError:
 * @DS_APPLICATION_ERROR_FAILED: generic error condition.
 * @DS_APPLICATION_ERROR_GSETTINGS_INIT: main settings initialization error.
 * @DS_APPLICATION_ERROR_LUA_INIT: Lua engine initialization error.
 * @DS_APPLICATION_ERROR_GLFW_INIT: GLFW3 initialization error.
 * @DS_APPLICATION_ERROR_GLFW_WINDOW_INIT: GLFW3 Window initialization error.
 * @DS_APPLICATION_ERROR_GLEW_INIT: glew initialization error.
 *
 * Error code returned by DsApplication API.
 * Note that %DS_APPLICATION_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_APPLICATION_ERROR_FAILED,
  DS_APPLICATION_ERROR_GSETTINGS_INIT,
  DS_APPLICATION_ERROR_LUA_INIT,
  DS_APPLICATION_ERROR_GLFW_INIT,
  DS_APPLICATION_ERROR_GLFW_WINDOW_INIT,
  DS_APPLICATION_ERROR_GL_CONTEXT_INIT,
  DS_APPLICATION_ERROR_GLEW_INIT,
} DsApplicationError;

#define DS_TYPE_APPLICATION             (ds_application_get_type())
#define DS_APPLICATION(object)          (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_APPLICATION, DsApplication))
#define DS_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_APPLICATION, DsApplicationClass))
#define DS_IS_APPLICATION(object)       (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_APPLICATION))
#define DS_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_APPLICATION))
#define DS_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_APPLICATION, DsApplicationClass))

typedef struct _DsApplication         DsApplication;
typedef struct _DsApplicationPrivate  DsApplicationPrivate;
typedef struct _DsApplicationClass    DsApplicationClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GQuark
ds_application_error_quark();
DEUSEXMAKINA2_API
GType
ds_application_get_type();

/**
 * DsApplication:
 * @parent_instance: parent instance.
 * @lua: Lua engine state.
 * @pipeline: application pipeline.
 * @renderer: application renderer.
 * @events: application events controller.
 *
 */
struct _DsApplication
{
  GApplication parent_instance;
  /*<private>*/
  DsApplicationPrivate* priv;
  /*<public>*/
  gpointer lua;
  DsPipeline* pipeline;
  DsRenderer* renderer;
  DsEvents* events;
};

/**
 * DsApplicationClass:
 * @parent_class: parent class.
 *
 */
struct _DsApplicationClass
{
  GApplicationClass parent_class;
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_APPLICATION_INCLUDED__
