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
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_marshals.h>
#include <SDL.h>

G_DEFINE_QUARK(ds-font-error-quark,
               ds_font_error);

static const gfloat   x_norm_ = 800.f;
static const gfloat   y_norm_ = 600.f;
static const gfloat   x_norm = x_norm_ / 2.f;
static const gfloat   y_norm = y_norm_ / 2.f;
static const guint64  x_spac = 2;

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
  gfloat x_adv;
  gfloat y_adv;
  vec2 size;
  vec2 start;
  vec2 uv;
  vec2 uv2;
};

struct _DsTextVertex
{
  vec2 xy;
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
typedef struct _DsTextVertex  DsTextVertex;

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
  DsCacheProvider* cache_provider;
  gint face_index;
  gint font_size;
  DsGlyph* glyphs;
  guint n_glyphs;
  guint64 image_w;
  guint64 image_h;
  GLuint tio;
};

struct _DsFontClass
{
  GObjectClass parent_class;

  /*<private>*/
  struct FT_MemoryRec_ ft_memory;
  FT_Library ft_library;
  gchar* charset;
  gfloat ddpi;
  gfloat hdpi;
  gfloat vdpi;
};

enum {
  prop_0,
  prop_font_file,
  prop_face_index,
  prop_font_size,
  prop_texture_id,
  prop_cache_provider,
  prop_number,
};

G_STATIC_ASSERT(sizeof(GLuint) == sizeof(guint));

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

/*
 * Calculate dpi
 *
 */

  int n_display = 0;
  int return_;

  return_ =
  SDL_GetDisplayDPI(n_display, &(klass->ddpi), &(klass->hdpi), &(klass->vdpi));
  if G_UNLIKELY(0 != return_)
  {
    g_set_error
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_SDL,
     "SDL_GetDisplayDPI(): failed!: %s\r\n",
     SDL_GetError());
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
  gboolean  success = TRUE;
  GError* tmp_err = NULL;
  GBytes* bytes = NULL;
  gchar* letter = NULL;
  FT_Error ft_error;
  gfloat font_size;
  guint i;

  DsFont* self = DS_FONT(pself);
  DsFontClass* klass = DS_FONT_GET_CLASS(pself);
  DsGlyph* glyphs;
  gsize n_glyphs;

  GLuint tio = 0;

  guint64 image_w = 0;
  guint64 image_h = 0;
  guint64 max_asc = 0;
  guint64 max_dec = 0;
  guint64 x_off = 0;
  guint64 y_off = 0;
  guint64 x, y;

  gfloat x_gsz;
  gfloat y_gsz;

/*
 * Get cache provider
 *
 */

  if G_UNLIKELY
    (self->cache_provider == NULL)
  {
    self->cache_provider =
    ds_cache_provider_get_default();

    if G_UNLIKELY
      (self->cache_provider == NULL)
    {
      g_set_error_literal
      (error,
       DS_FOLDER_PROVIDER_ERROR,
       DS_FOLDER_PROVIDER_ERROR_INVALID,
       "Invalid default cache provider\r\n");
      goto_error();
    }
    else
    {
      g_object_ref(self->cache_provider);
    }
  }

/*
 * Load font file bytes
 *
 */

  bytes =
  g_file_load_bytes(self->font_file, cancellable, NULL,&tmp_err);
  g_clear_object(&(self->font_file));
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

  font_size  = (gfloat) self->font_size;
  font_size *= 64.f;
  font_size *= 72.f / klass->ddpi;

  ft_error =
  FT_Set_Char_Size(face, 0, (FT_F26Dot6) font_size, (FT_UInt) klass->hdpi, (FT_UInt) klass->vdpi);
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

    image_w += glyph->bitmap.width + x_spac;

    if(glyph->bitmap_top > (gint64) max_asc)
      max_asc = (guint64) ( glyph->bitmap_top );
    if((glyph->metrics.height >> 6) - glyph->bitmap_top > (gint64) max_dec)
      max_dec = (guint64) ( (glyph->metrics.height >> 6) - glyph->bitmap_top );
  }

  image_h = max_asc + max_dec;

/*
 * Create GL texture
 *
 */

  __gl_try_catch(
    glGenTextures(1, &tio);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glBindTexture(GL_TEXTURE_2D, tio);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, image_w, image_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

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
    x_gsz = (gfloat) (glyph->metrics.width >> 6);
    y_gsz = (gfloat) (glyph->metrics.height >> 6);

    /* glyph advance */
    glyphs[i].x_adv = (gfloat) (glyph->advance.x >> 6);
    glyphs[i].y_adv = (gfloat) (glyph->advance.y >> 6);

    /* glyph size and bearings */
    glyphs[i].size[0] = x_gsz;
    glyphs[i].size[1] = y_gsz;
    glyphs[i].start[0] =  (gfloat) ((                        glyph->metrics.horiBearingX) >> 6);
    glyphs[i].start[1] = -(gfloat) ((glyph->metrics.height - glyph->metrics.horiBearingY) >> 6);

    /* texture size and bearings */
    glyphs[i]. uv[0] = ((gfloat) x_off) / ((gfloat) image_w);
    glyphs[i]. uv[1] = ((gfloat) y_off) / ((gfloat) image_h);
    glyphs[i].uv2[0] = (x_gsz / ((gfloat) image_w)) + glyphs[i].uv[0];
    glyphs[i].uv2[1] = (y_gsz / ((gfloat) image_h)) + glyphs[i].uv[1];

    __gl_try_catch(
      glTexSubImage2D(GL_TEXTURE_2D, 0, x_off, y_off, glyph->bitmap.width, glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    x_off += glyph->bitmap.width + x_spac;
  }

  __gl_try_catch(
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

/*
 * Complete
 *
 */

  self->glyphs = g_steal_pointer(&glyphs);
  self->n_glyphs = n_glyphs;
  self->image_w = image_w;
  self->image_h = image_h;
  self->tio = tio;
  tio = 0;

_error_:
  _g_free0(glyphs);
  _FT_Done_Face0(face);
  _g_bytes_unref0(bytes);
  if G_UNLIKELY(tio != 0)
    glDeleteTextures(1, &tio);
return success;
}

static
void ds_font_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_font_g_initable_iface_init_sync;
}

static
void ds_font_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(ds_font_new),
   ds_cclosure_marshal_OBJECT__OBJECT_INT_OBJECT_OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__OBJECT_INT_OBJECT_OBJECT_POINTERv,
   DS_TYPE_FONT,
   5,
   G_TYPE_FILE,
   G_TYPE_INT,
   DS_TYPE_CACHE_PROVIDER,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static
void ds_font_class_get_property(GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec) {
  DsFont* self = DS_FONT(pself);
  switch(prop_id)
  {
  case prop_texture_id:
    g_value_set_uint(value, self->tio);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
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
  case prop_cache_provider:
    g_set_object(&(self->cache_provider), g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_font_class_finalize(GObject* pself) {
  DsFont* self = DS_FONT(pself);
  glDeleteTextures(1, &(self->tio));
  _g_free0(self->glyphs);
G_OBJECT_CLASS(ds_font_parent_class)->finalize(pself);
}

static
void ds_font_class_dispose(GObject* pself) {
  DsFont* self = DS_FONT(pself);
  g_clear_object(&(self->font_file));
  g_clear_object(&(self->cache_provider));
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

  oclass->get_property = ds_font_class_get_property;
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

  properties[prop_texture_id] =
    g_param_spec_uint
    (_TRIPLET("texture-id"),
     0, G_MAXUINT, 0,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_cache_provider] =
    g_param_spec_object
    (_TRIPLET("cache-provider"),
     DS_TYPE_CACHE_PROVIDER,
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
ds_font_new(GFile            *font_file,
            gint              font_size,
            DsCacheProvider  *cache_provider,
            GCancellable     *cancellable,
            GError          **error)
{
  return (DsFont*)
  g_initable_new
  (DS_TYPE_FONT,
   cancellable,
   error,
   "font-file", font_file,
   "font-size", font_size,
   "cache-provider", cache_provider,
   NULL);
}

static inline DsGlyph*
search_glyph(DsGlyph* glyphs, guint n_glyphs, gunichar char_)
{
  guint i;
  for(i = 0;
      i < n_glyphs;
      i++)
  {
    if G_UNLIKELY(glyphs[i].glyph == char_)
    {
      return &(glyphs[i]);
    }
  }
return NULL;
}

#define n_vbox 6
typedef DsTextVertex vbox[n_vbox];

G_GNUC_INTERNAL
gboolean
_ds_font_generate_vao(DsFont         *font,
                      const gchar    *text,
                      vec2            xy,
                      GLuint         *pvao,
                      GLuint         *pvbo,
                      gsize          *pnvt,
                      GCancellable   *cancellable,
                      GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GLuint vao = 0;
  GLuint vbo = 0;

  const gchar* letter = NULL;
  DsGlyph* glyphs = font->glyphs;
  guint n_glyphs = font->n_glyphs;
  DsGlyph* glyph = NULL;
  gunichar char_;
  guint i, j;

  gsize length = g_utf8_strlen(text, -1);
  gsize n_vertices = length * 6;
  gsize b_vertices = sizeof(DsTextVertex) * n_vertices;
  vbox* boxes = NULL;

  gfloat height = 0.f;
  gfloat width = 0.f;
  gfloat x_off = xy[0], x_st;
  gfloat y_off = xy[1], y_st;

/*
 * Generate vertex array
 *
 */

  __gl_try_catch(
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glBindVertexArray(vao);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

/*
 * Create and map vertex buffer
 *
 */

  __gl_try_catch(
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, b_vertices, NULL, GL_STATIC_DRAW);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    boxes = (vbox*)
    glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

/*
 * Fill vertices
 *
 */

  for(letter = text, i = 0;
      letter != NULL && letter[0] != '\0';
      letter = g_utf8_next_char(letter), i++)
  {
    char_ =
    g_utf8_get_char(letter);
    switch(char_)
    {
    case '\n':
      y_off += (gfloat) font->image_h;
    case '\r':
      x_off = xy[0];
      continue;
    }

    glyph =
    search_glyph(glyphs, n_glyphs, char_);
    if G_UNLIKELY(glyph == NULL)
    {
      g_set_error
      (error,
       DS_FONT_ERROR,
       DS_FONT_ERROR_UNKNOWN_GLYPH,
       "Unknown glyph '%C'\r\n",
       char_);
      goto_error();
    }

    width = glyph->size[0];
    height = glyph->size[1];
    x_st = glyph->start[0];
    y_st = glyph->start[1];

  /*
   * Prepare square on
   * simple coordinates from
   * first character
   *
   */

    boxes[i][0].xy[0] = ((x_off + x_st         ) - x_norm) / x_norm;
    boxes[i][0].xy[1] = ((y_off + y_st + height) - y_norm) / y_norm;
    boxes[i][0].uv[0] = glyph->uv[0];
    boxes[i][0].uv[1] = glyph->uv[1];

    boxes[i][1].xy[0] = ((x_off + x_st         ) - x_norm) / x_norm;
    boxes[i][1].xy[1] = ((y_off + y_st         ) - y_norm) / y_norm;
    boxes[i][1].uv[0] = glyph->uv[0];
    boxes[i][1].uv[1] = glyph->uv2[1];

    boxes[i][2].xy[0] = ((x_off + x_st +  width) - x_norm) / x_norm;
    boxes[i][2].xy[1] = ((y_off + y_st         ) - y_norm) / y_norm;
    boxes[i][2].uv[0] = glyph->uv2[0];
    boxes[i][2].uv[1] = glyph->uv2[1];

    boxes[i][3].xy[0] = ((x_off + x_st         ) - x_norm) / x_norm;
    boxes[i][3].xy[1] = ((y_off + y_st + height) - y_norm) / y_norm;
    boxes[i][3].uv[0] = glyph->uv[0];
    boxes[i][3].uv[1] = glyph->uv[1];

    boxes[i][4].xy[0] = ((x_off + x_st +  width) - x_norm) / x_norm;
    boxes[i][4].xy[1] = ((y_off + y_st         ) - y_norm) / y_norm;
    boxes[i][4].uv[0] = glyph->uv2[0];
    boxes[i][4].uv[1] = glyph->uv2[1];

    boxes[i][5].xy[0] = ((x_off + x_st +  width) - x_norm) / x_norm;
    boxes[i][5].xy[1] = ((y_off + y_st + height) - y_norm) / y_norm;
    boxes[i][5].uv[0] = glyph->uv2[0];
    boxes[i][5].uv[1] = glyph->uv[1];

    x_off += glyph->x_adv;
  }

  __gl_try_catch(
    glUnmapBuffer(GL_ARRAY_BUFFER);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

  __gl_try_catch(
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DsTextVertex), (gconstpointer) G_STRUCT_OFFSET(DsTextVertex, xy));
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

  __gl_try_catch(
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DsTextVertex), (gconstpointer) G_STRUCT_OFFSET(DsTextVertex, uv));
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

_error_:
  glBindVertexArray(0);
  if G_UNLIKELY(success == FALSE)
  {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
  }
  else
  {
    *pvao = vao;
    *pvbo = vbo;
    *pnvt = n_vertices;
  }
return success;
}
