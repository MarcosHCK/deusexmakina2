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
 * Note that %DS_APPLICATION_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum
{
  DS_GL_ERROR_NO_ERROR = GL_NO_ERROR,                                             /*<nick=GL_NO_ERROR>*/
  DS_GL_ERROR_INVALID_ENUM = GL_INVALID_ENUM,                                     /*<nick=GL_INVALID_ENUM>*/
  DS_GL_ERROR_INVALID_VALUE = GL_INVALID_VALUE,                                   /*<nick=GL_INVALID_VALUE>*/
  DS_GL_ERROR_INVALID_OPERATION = GL_INVALID_OPERATION,                           /*<nick=GL_INVALID_OPERATION>*/
  DS_GL_ERROR_INVALID_FRAMEBUFFER_OPERATION = GL_INVALID_FRAMEBUFFER_OPERATION,   /*<nick=GL_INVALID_FRAMEBUFFER_OPERATION>*/
  DS_GL_ERROR_OUT_OF_MEMORY = GL_OUT_OF_MEMORY,                                   /*<nick=GL_OUT_OF_MEMORY>*/
  DS_GL_ERROR_STACK_UNDERFLOW = GL_STACK_UNDERFLOW,                               /*<nick=GL_STACK_UNDERFLOW>*/
  DS_GL_ERROR_STACK_OVERFLOW = GL_STACK_OVERFLOW,                                 /*<nick=GL_STACK_OVERFLOW>*/
} DsGLError;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_gl_error_get_type();

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

gboolean
ds_gl_has_error();
GError*
ds_gl_get_error();

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_GL_INCLUDED__
