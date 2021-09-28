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
#include <config.h>
#include <ds_application_private.h>
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_shader.h>

typedef struct _Binary        Binary;
typedef struct _BinaryHeader  BinaryHeader;

static
const gchar s_magic[4] = "DSS\x1b";

#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))

/*
 * Structs
 *
 */

#pragma pack(push, 1)
#define _PACKED __attribute__((packed, aligned(1)))

struct _BinaryHeader
{
  gchar magic[4];
  guint source_hash;
  guint format;
  guint length;
} _PACKED;

struct _Binary
{
  BinaryHeader header;
  guchar bytes[1];
} _PACKED;

#pragma pack(pop)

G_STATIC_ASSERT(sizeof(guint) == sizeof(GLenum));
G_STATIC_ASSERT(sizeof(guint) == sizeof(GLsizei));
G_STATIC_ASSERT(sizeof(guint) == sizeof(GLuint));
G_STATIC_ASSERT(sizeof(guint) == sizeof(GLint));

/*
 * API
 *
 */

static GFile*
get_shader_cache_dir(guint source_hash, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsApplication* app = NULL;
  GFile* return_ = NULL;
  GFile* basedir = NULL;
  GFile* shader = NULL;

/*
 * Generate name
 *
 */

  app = (DsApplication*)
  g_application_get_default();
  g_assert(DS_IS_APPLICATION(app));

  gchar buf[64];
  g_snprintf(buf, sizeof(buf), "%x.shader", source_hash);

  basedir =
  _ds_base_dirs_child("shaders", app->glcachedir, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  shader =
  g_file_get_child(basedir, buf);

/*
 * Finalize
 *
 */

  return_ = g_steal_pointer(&shader);

_error_:
  g_clear_object(&basedir);
  g_clear_object(&shader);
return return_;
}

static gboolean
load_from_blob(GLuint         pid,
               GBytes        *bytes,
               GCancellable  *cancellable,
               GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  BinaryHeader* header;
  GLint return_ = 0;
  GLint infolen = 0;
  gsize length = 0;

/*
 * Sanity checks
 *
 */

  header = (BinaryHeader*)
  g_bytes_get_data(bytes, &length);

  if G_UNLIKELY(0 >= length || header == NULL)
  {
    g_set_error_literal
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_INVALID_BINARY,
     "Invalid file size\r\n");
    goto_error();
  }

  if G_UNLIKELY(memcmp(header->magic, s_magic, sizeof(s_magic)) != 0)
  {
    g_set_error_literal
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_INVALID_BINARY,
     "Invalid file magic\r\n");
    goto_error();
  }

  if G_UNLIKELY(sizeof(*header) + header->length != length)
  {
    g_set_error_literal
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_INVALID_BINARY,
     "Invalid blob size\r\n");
    goto_error();
  }

/*
 * Load bytes
 *
 */

  __gl_try_catch(
    glProgramBinary(pid, (GLenum) header->format, (gconstpointer) &(header[1]), header->length);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glGetProgramiv(pid, GL_LINK_STATUS, &return_);
    glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &infolen);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_UNLIKELY(return_ == FALSE)
  {
    g_assert(infolen != 0);
    gchar* msg = g_malloc(infolen + 1);

    __gl_try_catch(
      glGetProgramInfoLog(pid, infolen, NULL, msg);
    ,
      g_propagate_error(error, glerror);
      _g_free0(msg);
      goto_error();
    );

    g_set_error_literal
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_INVALID_PROGRAM,
     msg);
    _g_free0(msg);
    goto_error();
  }

_error_:
return success;
}

G_GNUC_INTERNAL
gboolean
_ds_shader_cache_try_load(GLuint          pid,
                          guint           source_hash,
                          GCancellable   *cancellable,
                          GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GBytes* bytes = NULL;
  GFile* file = NULL;

/*
 * Load binary blob
 *
 */

  file =
  get_shader_cache_dir(source_hash, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  bytes =
  g_file_load_bytes(file, cancellable, NULL, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    success = (bytes != NULL);
    if(g_error_matches(tmp_err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
    {
      g_error_free(tmp_err);
      goto _error_;
    }
    else
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

/*
 * Load blob into program
 *
 */

  success =
  load_from_blob(pid, bytes, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    if G_LIKELY(tmp_err->domain == DS_GL_ERROR)
    {
      g_set_error_literal
      (error,
       DS_SHADER_ERROR,
       DS_SHADER_ERROR_INVALID_BINARY,
       tmp_err->message);
      g_clear_error(&tmp_err);
      goto_error();
    }
    else
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

_error_:
  _g_object_unref0(file);
  _g_bytes_unref0(bytes);
return success;
}

G_GNUC_INTERNAL
gboolean
_ds_shader_cache_try_save(GLuint          pid,
                          guint           source_hash,
                          GCancellable   *cancellable,
                          GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GOutputStream* stream = NULL;
  BinaryHeader header = {0};
  guchar* buff = NULL;
  GFile* file = NULL;
  GLenum format;
  GLsizei length;
  GLsizei wrote_;
  gsize wrote;

  memcpy(&(header.magic), &s_magic, sizeof(s_magic));

/*
 * Get program length
 *
 */

  __gl_try_catch(
    glGetProgramiv(pid, GL_PROGRAM_BINARY_LENGTH, &length);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_UNLIKELY(0 >= length)
  {
    g_set_error_literal
    (error,
     DS_GL_ERROR,
     DS_GL_ERROR_INVALID_VALUE,
     "Invalid binary length\r\n");
    goto_error();
  }
  else
  {
    buff = g_malloc(length);
  }

  header.length = length;

/*
 * Get program bytes
 *
 */

  __gl_try_catch(
    glGetProgramBinary(pid, length, &wrote_, &format, buff);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  header.format = format;

  if G_UNLIKELY(wrote_ != length)
  {
    g_set_error
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_FAILED,
     "(%s: %i) Truncated or overflowed output\r\n",
     G_STRFUNC,
     __LINE__);
    goto_error();
  }

/*
 * Write bytes
 *
 */

  /* generate name */
  file =
  get_shader_cache_dir(source_hash, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  /* open file */
  stream = (GOutputStream*)
  g_file_replace(file, NULL, FALSE, G_FILE_CREATE_PRIVATE, cancellable, &tmp_err);
  g_clear_object(&file);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  } else
  if G_UNLIKELY(G_IS_OUTPUT_STREAM(stream) == FALSE)
  {
    g_set_error
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_FAILED,
     "(%s: %i) Invalid stream object\r\n",
     G_STRFUNC,
     __LINE__);
    goto_error();
  }

  /* write header */
  success =
  g_output_stream_write_all(stream, &header, sizeof(header), &wrote, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  } else
  if G_UNLIKELY(wrote != sizeof(header))
  {
    g_set_error
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_FAILED,
     "(%s: %i) Truncated or overflowed output\r\n",
     G_STRFUNC,
     __LINE__);
    goto_error();
  }

  /* write bytes */
  success =
  g_output_stream_write_all(stream, buff, (gsize) length, &wrote, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  } else
  if(wrote != (gsize) length)
  {
    g_set_error
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_FAILED,
     "(%s: %i) Truncated or overflowed output\r\n",
     G_STRFUNC,
     __LINE__);
    goto_error();
  }

  /* close stream */
  g_output_stream_close(stream, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  _g_object_unref0(stream);
  _g_object_unref0(file);
  _g_free0(buff);
return success;
}

G_GNUC_INTERNAL
gboolean
_ds_shader_cache_cleanup(guint          source_hash,
                         GCancellable  *cancellable,
                         GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GFileEnumerator* enum_ = NULL;
  GFile* parent = NULL;
  GFile* child = NULL;
  GFile* file = NULL;
  guint i;

/*
 * Get current blob filename
 *
 */

  file =
  get_shader_cache_dir(source_hash, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Enumerate blob storage unit
 *
 */

  parent = g_file_get_parent(file);
  g_assert(parent != NULL);

  /* open enumerator */
  enum_ =
  g_file_enumerate_children(parent, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, cancellable, &tmp_err);
  g_clear_object(&parent);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  /* iterate */
  while(g_file_enumerator_iterate(enum_, NULL, &child, cancellable, &tmp_err))
  {
    if G_UNLIKELY(child == NULL)
      break;

    gboolean is =
    g_file_equal(file, child);
    if G_LIKELY(is == FALSE)
    {
      success =
      g_file_delete(child, cancellable, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
    }
  }

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  /* close enumerator */
  g_file_enumerator_close(enum_, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  _g_object_unref0(enum_);
  _g_object_unref0(parent);
  _g_object_unref0(file);
return success;
}
