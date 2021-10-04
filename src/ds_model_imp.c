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
#include <ds_model_private.h>

G_DEFINE_QUARK(ds-model-import-aifileio-quark,
               ds_model_imp_fio);
G_DEFINE_QUARK(ds-model-import-aifileio-data-quark,
               ds_model_imp_fio_data);

typedef struct _FioData FioData;

#define _fio_data_free0(var) ((var == NULL) ? NULL : (var = (_fio_data_free0 (var), NULL)))
#define _fileio_free0(var) ((var == NULL) ? NULL : (var = (_fileio_free0 (var), NULL)))

struct _FioData
{
  FioData* chain;

  union
  {
    GFile* source;
    GInputStream* stream;
    GSeekable* seekable;
    GObject* object;
  };

  GCancellable* cancellable;
  GError* error;
};

/*
 * FioData stuff
 *
 */

static void
push_error0(FioData* data, GError* src)
{
  if G_UNLIKELY
    (data->error != NULL)
  {
    GError* dst = data->error;

    g_critical
    ("Setting a new error over a previous one\r\n"
     "Previous was %s: %i: %s\r\n",
     "New is %s: %i: %s\r\n",
     g_quark_to_string(dst->domain),
     dst->code,
     dst->message,
     g_quark_to_string(src->domain),
     src->code,
     src->message);
    g_error_free(dst);
    data->error = src;
  }
  else
  {
    data->error = src;
  }
}

static void
push_error(FioData* data, GError* src)
{
  g_return_if_fail(src != NULL);
  if(data->chain != NULL)
    push_error0(data->chain, src);
  else
    push_error0(data, src);
}

static GError*
pop_error(FioData* data)
{
  if(data->chain != NULL)
    return g_steal_pointer(&(data->chain->error));
  else
    return g_steal_pointer(&(data->error));
return NULL;
}

static void
(_fio_data_free0)(FioData* data)
{
  if G_LIKELY(data != NULL)
  {
    g_clear_object(&(data->object));
    g_clear_object(&(data->cancellable));

    if G_UNLIKELY(data->error != NULL)
    {
      GError* tmp_err = data->error;

      g_critical
      ("Uncaught error: %s: %i: %s\r\n",
       g_quark_to_string(tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
    }

    g_slice_free(FioData, data);
  }
}

static void
(_fileio_free0)(C_STRUCT aiFileIO* fio)
{
  if G_LIKELY(fio != NULL)
    g_slice_free(C_STRUCT aiFileIO, fio);
}

/*
 * aiFileIO virtual functions
 *
 */

static size_t
ai_read_proc(C_STRUCT aiFile* ifile, char* buffer, size_t size, size_t count)
{
  FioData* data = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  gsize read = -1;

  g_input_stream_read_all
  (data->stream,
   buffer,
   (gsize)
   (size * count),
   &read,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data, tmp_err);
    goto_error();
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    return (size_t) (-1);
return (size_t) read;
}

static size_t
ai_tell_proc(C_STRUCT aiFile* ifile)
{
  FioData* data = (FioData*) ifile->UserData;
return (size_t) g_seekable_tell(data->seekable);
}

static size_t
ai_file_size_proc(C_STRUCT aiFile* ifile)
{
  FioData* data = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GFileInfo* info = NULL;
  gsize size = -1;

  info =
  g_file_input_stream_query_info
  (G_FILE_INPUT_STREAM(data->stream),
   G_FILE_ATTRIBUTE_STANDARD_SIZE,
   data->cancellable,
   &tmp_err);

  size =
  g_file_info_get_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_STANDARD_SIZE);

_error_:
  _g_object_unref0(info);
return (size_t) size;
}

static C_ENUM aiReturn
ai_seek_proc(C_STRUCT aiFile* ifile, size_t offset, C_ENUM aiOrigin from)
{
  FioData* data = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GSeekType type = 0;

  switch(from)
  {
  case aiOrigin_SET:
    type = G_SEEK_SET;
    break;
  case aiOrigin_CUR:
    type = G_SEEK_CUR;
    break;
  case aiOrigin_END:
    type = G_SEEK_END;
    break;
  default:
    g_assert_not_reached();
    break;
  }

  g_seekable_seek
  (data->seekable,
   offset,
   type,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data, tmp_err);
    goto_error();
  }

_error_:
return success ? aiReturn_SUCCESS : aiReturn_FAILURE;
}

static C_STRUCT aiFile*
ai_open_proc(C_STRUCT aiFileIO* fio, const gchar* path, const gchar* mode)
{
  FioData* data = (FioData*) fio->UserData;
  C_STRUCT aiFile* ifile = NULL;
  GInputStream* stream = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  g_assert(mode[0] == 'r');

/*
 * Open file
 *
 */

  GFile* file =
  g_file_get_child(data->source, path);

  stream = (GInputStream*)
  g_file_read(file, data->cancellable, &tmp_err);
  g_object_unref(file);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data, tmp_err);
    goto_error();
  }

/*
 * Prepare aiFile structure
 *
 */

  FioData* data2 = g_slice_new(FioData);
  ifile = g_slice_new(C_STRUCT aiFile);
  ifile->UserData = (aiUserData) data2;

  data2->chain = data;
  data2->stream = g_steal_pointer(&stream);
  data2->cancellable = _g_object_ref0(data->cancellable);
  data2->error = NULL;

  ifile->ReadProc = ai_read_proc;
  ifile->WriteProc = NULL;
  ifile->TellProc = ai_tell_proc;
  ifile->FileSizeProc = ai_file_size_proc;
  ifile->SeekProc = ai_seek_proc;
  ifile->FlushProc = NULL;

_error_:
  g_clear_object(&stream);
  if G_UNLIKELY(success == FALSE)
  {
    if(ifile != NULL)
    {
      if(ifile->UserData)
      {
        g_clear_pointer
        ((FioData**)
         &(ifile->UserData),
         _fio_data_free0);
      }

      g_slice_free
      (C_STRUCT aiFile,
       ifile);
      ifile = NULL;
    }
  }
return ifile;
}

static void
aio_close_proc(C_STRUCT aiFileIO* fio, C_STRUCT aiFile* ifile)
{
  FioData* data = (FioData*) fio->UserData;
  FioData* data2 = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  g_input_stream_close
  (data2->stream,
   data2->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data2, tmp_err);
  }
}

/*
 * Entry
 *
 */

G_GNUC_INTERNAL
const C_STRUCT aiScene*
_ds_model_import_new(DsModel* self, GFile* source, const gchar* name, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  const C_STRUCT aiScene* scene = NULL;

  C_STRUCT aiFileIO* fio = NULL;
  FioData* data = NULL;

  fio = g_slice_new0(C_STRUCT aiFileIO);
  data = g_slice_new0(FioData);

  data->source = _g_object_ref0(source);
  data->cancellable = _g_object_ref0(cancellable);

  fio->OpenProc = ai_open_proc;
  fio->CloseProc = aio_close_proc;
  fio->UserData = (aiUserData) data;

  scene =
  aiImportFileEx
  (name,
     aiProcess_CalcTangentSpace
   | aiProcess_GenSmoothNormals
   | aiProcess_GenUVCoords
   | aiProcess_JoinIdenticalVertices
   | aiProcess_OptimizeGraph
   | aiProcess_OptimizeMeshes
   | aiProcess_Triangulate,
   fio);

  tmp_err = pop_error(data);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  if G_UNLIKELY
    (scene == NULL
     || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
     || (scene->mRootNode == NULL))
  {
    g_set_error_literal
    (error,
     DS_MODEL_ERROR,
     DS_MODEL_ERROR_INCOMPLETE_IMPORT,
     "Incomplete import\r\n");
    goto_error();
  }

_error_:
  if G_UNLIKELY(success == FALSE)
  {
    _aiReleaseImport0(scene);
    _fio_data_free0(data);
    _fileio_free0(fio);
  }
  else
  {
    g_object_set_qdata_full
    (G_OBJECT(self),
     ds_model_imp_fio_data_quark(),
     data,
     (GDestroyNotify)
     _fio_data_free0);
    g_object_set_qdata_full
    (G_OBJECT(self),
     ds_model_imp_fio_quark(),
     fio,
     (GDestroyNotify)
     _fileio_free0);
  }
return scene;
}

G_GNUC_INTERNAL
void
_ds_model_import_free(DsModel* self, const C_STRUCT aiScene* scene)
{
  C_STRUCT aiFileIO* fio = NULL;
  FioData* data = NULL;

  if G_LIKELY(scene != NULL)
  {
    data = g_object_steal_qdata(G_OBJECT(self), ds_model_imp_fio_data_quark());
    fio = g_object_steal_qdata(G_OBJECT(self), ds_model_imp_fio_quark());

    _aiReleaseImport0(scene);
    _fio_data_free0(data);
    _fileio_free0(fio);
  }
}
