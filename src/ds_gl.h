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
#ifndef __DS_GL_INCLUDED__
#define __DS_GL_INCLUDED__
#include <glib-object.h>
#include <GL/glew.h>

/**
 * DS_GL_ERROR:
 *
 * Error domain for OpenGL errors. Errors in this domain will be from the #DsGLError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_GL_ERROR (ds_gl_error_quark())

/**
 * DsGLError:
 * @DS_GL_ERROR_FAILED: not a GL error but an error code nevertheless,
 * it may represent an error on application using GL.
 * @DS_GL_ERROR_NO_ERROR: no error has been recorded.
 * @DS_GL_ERROR_INVALID_ENUM: an unacceptable value is specified for an enumerated argument.
 * @DS_GL_ERROR_INVALID_VALUE: A numeric argument is out of range.
 * @DS_GL_ERROR_INVALID_OPERATION: The specified operation is not allowed in the current state.
 * @DS_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.
 * @DS_GL_ERROR_OUT_OF_MEMORY: There is not enough memory left to execute the command.
 * @DS_GL_ERROR_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.
 * @DS_GL_ERROR_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.
 *
 * Error code returned by DsApplication API.
 * Note that %DS_GL_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum
{
  DS_GL_ERROR_FAILED = 0,                                                         /*<nick=DS_GL_ERROR_FAILED>*/
  DS_GL_ERROR_NO_ERROR = GL_NO_ERROR,                                             /*<nick=GL_NO_ERROR>*/
  DS_GL_ERROR_INVALID_ENUM = GL_INVALID_ENUM,                                     /*<nick=GL_INVALID_ENUM>*/
  DS_GL_ERROR_INVALID_VALUE = GL_INVALID_VALUE,                                   /*<nick=GL_INVALID_VALUE>*/
  DS_GL_ERROR_INVALID_OPERATION = GL_INVALID_OPERATION,                           /*<nick=GL_INVALID_OPERATION>*/
  DS_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION = GL_INVALID_FRAMEBUFFER_OPERATION,   /*<nick=GL_INVALID_FRAMEBUFFER_OPERATION>*/
  DS_GL_ERROR_OUT_OF_MEMORY = GL_OUT_OF_MEMORY,                                   /*<nick=GL_OUT_OF_MEMORY>*/
  DS_GL_ERROR_STACK_UNDERFLOW = GL_STACK_UNDERFLOW,                               /*<nick=GL_STACK_UNDERFLOW>*/
  DS_GL_ERROR_STACK_OVERFLOW = GL_STACK_OVERFLOW,                                 /*<nick=GL_STACK_OVERFLOW>*/
} DsGLError;

/**
 * DsGLError:
 * @DS_GL_OBJECT_VERTEX_ARRAY: vertex array object.
 *
 * OpenGL objects.
 */
typedef enum
{
  DS_GL_OBJECT_VERTEX_ARRAY = GL_VERTEX_ARRAY,  /*<nick=GL_VERTEX_ARRAY>*/
} DsGLObjectType;

/**
 * DsGLTextureType:
 * @DS_GL_TEXTURE_TYPE_NONE: empty texture.
 * @DS_GL_TEXTURE_TYPE_DIFFUSE: diffuse texture.
 * @DS_GL_TEXTURE_TYPE_SPECULAR: specular texture.
 * @DS_GL_TEXTURE_TYPE_NORMAL: normals texture.
 * @DS_GL_TEXTURE_TYPE_HEIGHT: heights texture.
 *
 * OpenGL textures.
 */
typedef enum
{
  DS_GL_TEXTURE_TYPE_NONE,
  DS_GL_TEXTURE_TYPE_DIFFUSE ,
  DS_GL_TEXTURE_TYPE_SPECULAR,
  DS_GL_TEXTURE_TYPE_NORMAL,
  DS_GL_TEXTURE_TYPE_HEIGHT,
} DsGLTextureType;

/**
 * DsGLDebugSource:
 * @DS_GL_DEBUG_SOURCE_API: message comes from API functions.
 * @DS_GL_DEBUG_SOURCE_WINDOW_SYSTEM: message comes from window system.
 * @DS_GL_DEBUG_SOURCE_SHADER_COMPILER: message comes from shader compiler.
 * @DS_GL_DEBUG_SOURCE_THIRD_PARTY: message comes from a third party source.
 * @DS_GL_DEBUG_SOURCE_APPLICATION: message comes from application.
 * @DS_GL_DEBUG_SOURCE_OTHER: message comes from other sources.
 *
 * OpenGL debug sources.
 */
typedef enum
{
  DS_GL_DEBUG_SOURCE_API = GL_DEBUG_SOURCE_API,                         /*<nick=GL_DEBUG_SOURCE_API>*/
  DS_GL_DEBUG_SOURCE_WINDOW_SYSTEM = GL_DEBUG_SOURCE_WINDOW_SYSTEM,     /*<nick=GL_DEBUG_SOURCE_WINDOW_SYSTEM>*/
  DS_GL_DEBUG_SOURCE_SHADER_COMPILER = GL_DEBUG_SOURCE_SHADER_COMPILER, /*<nick=GL_DEBUG_SOURCE_SHADER_COMPILER>*/
  DS_GL_DEBUG_SOURCE_THIRD_PARTY = GL_DEBUG_SOURCE_THIRD_PARTY,         /*<nick=GL_DEBUG_SOURCE_THIRD_PARTY>*/
  DS_GL_DEBUG_SOURCE_APPLICATION = GL_DEBUG_SOURCE_APPLICATION,         /*<nick=GL_DEBUG_SOURCE_APPLICATION>*/
  DS_GL_DEBUG_SOURCE_OTHER = GL_DEBUG_SOURCE_OTHER,                     /*<nick=GL_DEBUG_SOURCE_OTHER>*/
} DsGLDebugSource;

/**
 * DsGLDebugMessageType:
 * @DS_GL_DEBUG_MESSAGE_TYPE_ERROR: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_DEPRECATED_BEHAVIOR: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_UNDEFINED_BEHAVIOR: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_PORTABILITY: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_PERFORMANCE: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_OTHER: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_MARKER: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_PUSH_GROUP: self explanatory.
 * @DS_GL_DEBUG_MESSAGE_TYPE_POP_GROUP: self explanatory.
 *
 * OpenGL debug message types.
 */
typedef enum
{
  DS_GL_DEBUG_MESSAGE_TYPE_ERROR = GL_DEBUG_TYPE_ERROR,                             /*<nick=GL_DEBUG_TYPE_ERROR>*/
  DS_GL_DEBUG_MESSAGE_TYPE_DEPRECATED_BEHAVIOR = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, /*<nick=GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR>*/
  DS_GL_DEBUG_MESSAGE_TYPE_UNDEFINED_BEHAVIOR = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,   /*<nick=GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR>*/
  DS_GL_DEBUG_MESSAGE_TYPE_PORTABILITY = GL_DEBUG_TYPE_PORTABILITY,                 /*<nick=GL_DEBUG_TYPE_PORTABILITY>*/
  DS_GL_DEBUG_MESSAGE_TYPE_PERFORMANCE = GL_DEBUG_TYPE_PERFORMANCE,                 /*<nick=GL_DEBUG_TYPE_PERFORMANCE>*/
  DS_GL_DEBUG_MESSAGE_TYPE_OTHER = GL_DEBUG_TYPE_OTHER,                             /*<nick=GL_DEBUG_TYPE_OTHER>*/
  DS_GL_DEBUG_MESSAGE_TYPE_MARKER = GL_DEBUG_TYPE_MARKER,                           /*<nick=GL_DEBUG_TYPE_MARKER>*/
  DS_GL_DEBUG_MESSAGE_TYPE_PUSH_GROUP = GL_DEBUG_TYPE_PUSH_GROUP,                   /*<nick=GL_DEBUG_TYPE_PUSH_GROUP>*/
  DS_GL_DEBUG_MESSAGE_TYPE_POP_GROUP = GL_DEBUG_TYPE_POP_GROUP,                     /*<nick=GL_DEBUG_TYPE_POP_GROUP>*/
} DsGLDebugMessageType;

/**
 * DsGLDebugSeverityType:
 * @DS_GL_DEBUG_SEVERITY_LOW: self explanatory.
 * @DS_GL_DEBUG_SEVERITY_MEDIUM: self explanatory.
 * @DS_GL_DEBUG_SEVERITY_HIGH: self explanatory.
 * @DS_GL_DEBUG_SEVERITY_NOTIFICATION: self explanatory.
 *
 * OpenGL debug message severities.
 */
typedef enum
{
  DS_GL_DEBUG_SEVERITY_LOW = GL_DEBUG_SEVERITY_LOW,                   /*<nick=GL_DEBUG_SEVERITY_LOW>*/
  DS_GL_DEBUG_SEVERITY_MEDIUM = GL_DEBUG_SEVERITY_MEDIUM,             /*<nick=GL_DEBUG_SEVERITY_MEDIUM>*/
  DS_GL_DEBUG_SEVERITY_HIGH = GL_DEBUG_SEVERITY_HIGH,                 /*<nick=GL_DEBUG_SEVERITY_HIGH>*/
  DS_GL_DEBUG_SEVERITY_NOTIFICATION = GL_DEBUG_SEVERITY_NOTIFICATION, /*<nick=GL_DEBUG_SEVERITY_NOTIFICATION>*/
} DsGLDebugSeverityType;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GQuark
ds_gl_error_quark();
GType
ds_gl_error_get_type();
GType
ds_gl_object_type_get_type();
GType
ds_gl_texture_type_get_type();
GType
ds_gl_debug_type_get_type();
GType
ds_gl_debug_source_get_type();
GType
ds_gl_debug_message_type_get_type();
GType
ds_gl_debug_severity_get_type();

#define __gl_try(code) \
  G_STMT_START { \
    code ; \
  } G_STMT_END; \

#define __gl_catch(code, finally) \
  G_STMT_START { \
    if G_UNLIKELY(ds_gl_has_error() == TRUE) \
    { \
      GError* glerror = ds_gl_get_error(); \
      code ; \
    } \
    else \
    { \
      finally ; \
    } \
  } G_STMT_END

#define __gl_try_catch(code, catch) \
  G_STMT_START { \
    __gl_try(code); \
    __gl_catch(catch,); \
  } G_STMT_END

gboolean
ds_gl_has_error();
GError*
ds_gl_get_error();

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_GL_INCLUDED__
