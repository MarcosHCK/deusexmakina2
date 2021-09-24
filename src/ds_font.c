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
#include <cglm/cglm.h>
#include <ds_callable.h>
#include <ds_font.h>
#include <ds_macros.h>
#include <ds_marshals.h>

G_DEFINE_QUARK(ds-font-error-quark,
               ds_font_error);

/*
 * FreeType2
 *
 */

/* Compile-time options */
#define FT_ERROR_START_LIST \
  static const \
  struct \
  { \
    gint          code; \
    const gchar*  literal; \
  } ft_errors[] = {
#define FT_ERRORDEF(e,v,s) \
    { (gint) (v), s },
#define FT_ERROR_END_LIST \
  };

#define FT_MODERR_START_LIST \
  static const \
  struct \
  { \
    gint          code; \
    const gchar*  literal; \
  } ft_modules[] = {
#define FT_MODERRDEF(e,v,s) \
    { (gint) (v), s },
#define FT_MODERR_END_LIST \
  };

#define _FT_OK(c) (((c)) == 0)

/* headers*/
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H
#include FT_MODULE_H
#include FT_MODULE_ERRORS_H
#include FT_SYSTEM_H

/*
 * Structures
 *
 */

struct _DsGlyph
{
  gunichar glyph;
  vec2 uv;
};

/*
 * Object callbacks
 *
 */

static
void ds_font_class_base_init(DsFontClass* klass);
static
void ds_font_class_base_fini(DsFontClass* klass);
static
void ds_font_class_init(DsFontClass* klass);
static
void ds_font_init(DsFont* self);
static
void ds_font_g_initable_iface_init(GInitableIface* iface);
static
void ds_font_ds_callable_iface_init(DsCallableIface* iface);
static
gpointer ds_font_parent_class = NULL;

typedef struct _DsGlyph       DsGlyph;
typedef struct _DsWriterData  DsWriterData;

#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))
#define _FT_Done_Face0(var) ((var == NULL) ? NULL : (var = (FT_Done_Face (var), NULL)))

/*
 * Object definition
 *
 */

struct _DsFont
{
  GObject parent_instance;

  /*<private>*/
  GFile* font_file;
  gint face_index;
  gint font_size;
  DsGlyph* glyphs;
  guint n_glyphs;
  guchar* charmap;
  guint64 charmapsz;
};

struct _DsFontClass
{
  GObjectClass parent_class;

  /*<private>*/
  struct FT_MemoryRec_ ft_memory;
  FT_Library ft_library;
  gchar* charset;
};

enum {
  prop_0,
  prop_font_file,
  prop_face_index,
  prop_font_size,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

GType
ds_font_get_type()
{
  static
  GType font_type = 0;

  if G_UNLIKELY(font_type == 0)
  {
    const
    GTypeInfo type_info =
    {
      sizeof(DsFontClass),
      (GBaseInitFunc)
      ds_font_class_base_init,
      (GBaseFinalizeFunc)
      ds_font_class_base_fini,
      (GClassInitFunc)
      ds_font_class_init,
      NULL,
      NULL,
      sizeof(DsFont),
      0,
      (GInstanceInitFunc)
      ds_font_init,
      NULL,
    };

    font_type =
    g_type_register_static
    (G_TYPE_OBJECT,
     g_intern_static_string
     ("DsFont"),
     &type_info,
     0);

#define g_define_type_id font_type

    G_IMPLEMENT_INTERFACE
    (G_TYPE_INITABLE,
     ds_font_g_initable_iface_init)

    G_IMPLEMENT_INTERFACE
    (DS_TYPE_CALLABLE,
     ds_font_ds_callable_iface_init)

#undef g_define_type_id
  }
return font_type;
}

static void*
ft_alloc(FT_Memory memory, long size)
{
  return g_malloc(size);
}

static void
ft_free(FT_Memory memory, void* block)
{
  return g_free(block);
}

static void*
ft_realloc(FT_Memory memory, long cur, long new_, void* block)
{
  return g_realloc(block, new_);
}

static GError*
ft_get_error(FT_Error ft_error)
{
  FT_Error base = FT_ERROR_BASE(ft_error);
  FT_Error module = FT_ERROR_MODULE(ft_error);

  g_return_val_if_fail(base > 0 && base < G_N_ELEMENTS(ft_errors), NULL);
  g_return_val_if_fail(module > 0 && module < G_N_ELEMENTS(ft_modules), NULL);

  return
  g_error_new
  (DS_FONT_ERROR,
   DS_FONT_ERROR_FREETYPE,
   "(%s: %i): %i: %s\r\n",
   ft_modules[module].literal,
   ft_modules[module].code,
   ft_errors[module].code,
   ft_errors[module].literal);
}

static gboolean
ds_font_class_base_init_(DsFontClass   *klass,
                         GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  FT_Error ft_error = 0;

  /*
 * Prepare 'ft_memory'
 *
 */

  klass->ft_memory.user = klass;
  klass->ft_memory.alloc = ft_alloc;
  klass->ft_memory.free = ft_free;
  klass->ft_memory.realloc = ft_realloc;

/*
 * Create library object
 *
 */

  ft_error =
  FT_New_Library(&(klass->ft_memory), &(klass->ft_library));
  if G_UNLIKELY
    (_FT_OK(ft_error) == FALSE)
  {
    g_propagate_error(error, ft_get_error(ft_error));
    goto_error();
  }

/*
 * Initialize library object
 *
 */

  FT_Set_Default_Properties(klass->ft_library);
  FT_Add_Default_Modules(klass->ft_library);

/*
 * Initialize charset
 *
 */

  success =
  _ds_icu_get_charset(&(klass->charset), NULL, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  g_utf8_validate(klass->charset, -1, NULL);
  if G_UNLIKELY(success == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_FAILED,
     "Charset contains invalid UTF8 codes\r\n");
    goto_error();
  }

_error_:
return success;
}

static
void ds_font_class_base_init(DsFontClass* klass) {
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  ds_font_class_base_init_(klass, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_critical
    ("(%s:%i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free(tmp_err);
    g_assert_not_reached();
  }
}

static
void ds_font_class_base_fini(DsFontClass* klass) {
  FT_Done_Library(klass->ft_library);
  klass->ft_library = NULL;
  _g_free0(klass->charset);
  klass->charset = NULL;
}

static gboolean
ds_font_g_initable_iface_init_sync(GInitable     *pself,
                                   GCancellable  *cancellable,
                                   GError       **error)
{
  DsFont* self = DS_FONT(pself);
  gboolean  success = TRUE;
  GError* tmp_err = NULL;
  GBytes* bytes = NULL;
  gchar* letter = NULL;
  FT_Error ft_error;
  guchar ucode;
  guint i;

  DsFontClass* klass = DS_FONT_GET_CLASS(self);
  DsGlyph* glyphs;
  gsize n_glyphs;

  guchar* image_px = NULL;
  guint64 image_sz = 0;
  guint64 image_w = 0;
  guint64 image_h = 0;
  guint64 max_asc = 0;
  guint64 max_dec = 0;
  guint64 x_off = 0;
  guint64 y_off = 0;
  guint64 x, y;

/*
 * Load font file bytes
 *
 */

  bytes =
  g_file_load_bytes(self->font_file, cancellable, NULL,&tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  gsize bytesz;
  gconstpointer bytes_ = g_bytes_get_data(bytes, &bytesz);
  FT_Face face = NULL;

/*
 * Open font file
 *
 */

  ft_error =
  FT_New_Memory_Face(klass->ft_library, bytes_, bytesz, self->face_index, &face);
  if G_UNLIKELY(_FT_OK(ft_error) == FALSE)
  {
    g_propagate_error(error, ft_get_error(ft_error));
    goto_error();
  }

/*
 * Scan font
 *
 */

  ft_error =
  FT_Set_Char_Size(face, 0, self->font_size * 64, 300, 300);
  if G_UNLIKELY(_FT_OK(ft_error) == FALSE)
  {
    g_propagate_error(error, ft_get_error(ft_error));
    goto_error();
  }

  FT_GlyphSlot glyph = face->glyph;
  n_glyphs = g_utf8_strlen(klass->charset, -1);
  glyphs = g_new0(DsGlyph, n_glyphs);

  for(letter = klass->charset, i = 0;
      letter != NULL && *letter != '\0';
      letter = g_utf8_next_char(letter), i++)
  {
    glyphs[i].glyph = g_utf8_get_char(letter);

    ft_error =
    FT_Load_Char(face, glyphs[i].glyph, FT_LOAD_RENDER);
    if G_UNLIKELY(_FT_OK(ft_error) == FALSE)
    {
      g_set_error_literal
      (error,
       DS_FONT_ERROR,
       DS_FONT_ERROR_FREETYPE,
       "FT_Load_Char(): failed!\r\n");
      goto_error();
    }

    glyphs[i].uv[0] = (gfloat) image_w;
    image_w += glyph->bitmap.width;

    if(glyph->bitmap.rows > (gint64) image_h)
      image_h = (guint64) ( glyph->bitmap.rows );
    if(glyph->bitmap_top > (gint64) max_asc)
      max_asc = (guint64) ( glyph->bitmap_top );
    if((glyph->metrics.height >> 6) - glyph->bitmap_top > (gint64) max_dec)
      max_dec = (guint64) ( (glyph->metrics.height >> 6) - glyph->bitmap_top );
  }

  image_h = max_asc + max_dec;
  image_sz = image_w * image_h * sizeof(*image_px);
  image_px = (gpointer) g_malloc0(image_sz);

  for(i = 0;
      i < n_glyphs;
      i++)
  {
    ft_error =
    FT_Load_Char(face, glyphs[i].glyph, FT_LOAD_RENDER);
    if G_UNLIKELY(_FT_OK(ft_error) == FALSE)
    {
      g_set_error_literal
      (error,
       DS_FONT_ERROR,
       DS_FONT_ERROR_FREETYPE,
       "FT_Load_Char(): failed!\r\n");
      goto_error();
    }

    y_off = max_asc - glyph->bitmap_top;

    for(x = 0;x < glyph->bitmap.width;x++)
    for(y = 0;y < glyph->bitmap.rows;y++)
    {
      guint64 image_idx = (x + x_off) + (y + y_off) * image_w;
      guint64 bitmap_idx = x + y * glyph->bitmap.width;
      image_px[image_idx] = glyph->bitmap.buffer[bitmap_idx];
    }

    x_off += glyph->bitmap.width;
  }

  self->glyphs = g_steal_pointer(&glyphs);
  self->n_glyphs = n_glyphs;
  self->charmap = g_steal_pointer(&image_px);
  self->charmapsz = image_sz;

_error_:
  g_clear_object(&(self->font_file));
  _g_bytes_unref0(bytes);
  _FT_Done_Face0(face);
  _g_free0(image_px);
  _g_free0(glyphs);
return success;
}

static
void ds_font_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_font_g_initable_iface_init_sync;
}

static DsFont*
_callable_new(gpointer        null_,
              GFile          *font_file,
              gint            font_size,
              GCancellable   *cancellable,
              GError        **error)
{
  return
  ds_font_new
  (font_file,
   font_size,
   cancellable,
   error);
}

static DsFont*
_callable_new_simple(gpointer       null_,
                     const gchar   *font_filename,
                     gint           font_size,
                     GCancellable  *cancellable,
                     GError       **error)
{
  GFile* font_file = NULL;
  font_file = g_file_new_for_path(font_filename);

  DsFont* font =
  ds_font_new
  (font_file,
   font_size,
   cancellable,
   error);

  g_object_unref(font_file);
return font;
}

static
void ds_font_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new),
   ds_cclosure_marshal_OBJECT__OBJECT_INT_OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__OBJECT_INT_OBJECT_POINTERv,
   DS_TYPE_FONT,
   4,
   G_TYPE_FILE,
   G_TYPE_INT,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
  ds_callable_iface_add_method
  (iface,
   "new_simple",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new_simple),
   ds_cclosure_marshal_OBJECT__STRING_INT_OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__STRING_INT_OBJECT_POINTERv,
   DS_TYPE_FONT,
   4,
   G_TYPE_STRING,
   G_TYPE_INT,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static
void ds_font_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  DsFont* self = DS_FONT(pself);
  switch(prop_id)
  {
  case prop_font_file:
    g_set_object(&(self->font_file), g_value_get_object(value));
    break;
  case prop_face_index:
    self->face_index = g_value_get_int(value);
    break;
  case prop_font_size:
    self->font_size = g_value_get_int(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_font_class_finalize(GObject* pself) {
  DsFont* self = DS_FONT(pself);
  _g_free0(self->glyphs);
  _g_free0(self->charmap);
G_OBJECT_CLASS(ds_font_parent_class)->finalize(pself);
}

static
void ds_font_class_dispose(GObject* pself) {
  DsFont* self = DS_FONT(pself);
  g_clear_object(&(self->font_file));
G_OBJECT_CLASS(ds_font_parent_class)->dispose(pself);
}

static
void ds_font_class_init(DsFontClass* klass) {
  ds_font_parent_class = g_type_class_peek_parent(klass);
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */

  oclass->set_property = ds_font_class_set_property;
  oclass->finalize = ds_font_class_finalize;
  oclass->dispose = ds_font_class_dispose;

/*
 * Properties
 *
 */

  properties[prop_font_file] =
    g_param_spec_object
    (_TRIPLET("font-file"),
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_face_index] =
    g_param_spec_int
    (_TRIPLET("face-index"),
     0, G_MAXINT, 0,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_font_size] =
    g_param_spec_int
    (_TRIPLET("font-size"),
     0, G_MAXUINT8, 0,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static
void ds_font_init(DsFont* self) {
}

/*
 * Object methods
 *
 */

DsFont*
ds_font_new(GFile          *font_file,
            gint            font_size,
            GCancellable   *cancellable,
            GError        **error)
{
  return (DsFont*)
  g_initable_new
  (DS_TYPE_FONT,
   cancellable,
   error,
   "font-file", font_file,
   "font-size", font_size,
   NULL);
}
