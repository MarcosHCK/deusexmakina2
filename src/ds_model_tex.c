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

/*
 * Boxed define
 *
 */

static
gboolean _check = FALSE;
static const
guint n_tios = G_N_ELEMENTS(gl2ai);

G_DEFINE_BOXED_TYPE
(DsModelTexture,
 ds_model_texture,
 ds_model_texture_ref,
 ds_model_texture_unref);

DsModelTexture*
ds_model_texture_new(GError** error)
{
  /* check texture units */
  if G_UNLIKELY(_check == FALSE)
  {
    __gl_try_catch(
      GLint tui = 0;
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tui);
      if G_UNLIKELY(tui < n_tios)
      {
        /* only in broken GL implementations */
        g_critical("Too few image units supported\r\n");
        g_assert_not_reached();
      }
    ,
      g_propagate_error(error, glerror);
      return NULL;
    );

    _check = TRUE;
  }

  /* allocate */
  DsModelTexture* tex = NULL;
  tex = g_slice_new0(DsModelTexture);
  g_ref_count_init(&(tex->refs));

  /* generate textures */
  __gl_try_catch(
    glGenTextures(n_tios, tex->tios);
  ,
    g_propagate_error(error, glerror);
    goto _error_;
  );

  ds_model_texture_ref(tex);
_error_:
  ds_model_texture_unref(tex);
return tex;
}

DsModelTexture*
ds_model_texture_ref(DsModelTexture* tex)
{
  g_return_val_if_fail(tex != NULL, NULL);
  g_ref_count_inc(&(tex->refs));
return tex;
}

void
ds_model_texture_unref(DsModelTexture* tex)
{
  g_return_if_fail(tex != NULL);

  gboolean zero =
  g_ref_count_dec(&(tex->refs));
  if(zero == TRUE)
  {
    __gl_try_catch(
      glDeleteTextures(n_tios, tex->tios);
    ,
      g_warning
      ("(%s: %i): %s: %i: %s\r\n",
       G_STRFUNC,
       __LINE__,
       g_quark_to_string(glerror->domain),
       glerror->code,
       glerror->message);
      g_error_free(glerror);
    );

    g_slice_free(DsModelTexture, tex);
  }
}

G_GNUC_INTERNAL
void
_ds_model_compile_switch_texture(DsModelTexture* tex, DsRenderState* state)
{
  g_return_if_fail(tex != NULL);
  g_return_if_fail(state != NULL);

#if GL_VERSION_4_4 == 1
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindTextures),
   3,
   (guintptr) 0,
   (guintptr) n_tios,
   (guintptr) tex->tios);
#else
  GLuint tui;
  guint i;
# if GL_ARB_direct_state_access == 1
  if G_LIKELY(GLEW_ARB_direct_state_access == TRUE)
  {
    for(i = 0, tui = 0;
        i < n_tios;
        i++)
    {
      ds_render_state_pcall
      (state,
       G_CALLBACK(glBindTextureUnit),
       2,
       (guintptr) tui++,
       (guintptr) tex->tios[i]);
    }
  }
  else
  {
# endif // GL_ARB_direct_state_access
    for(i = 0, tui = GL_TEXTURE0;
        i < n_tios;
        i++)
    {
      ds_render_state_pcall
      (state,
       G_CALLBACK(glActiveTexture),
       1,
       (guintptr) tui++);

      ds_render_state_pcall
      (state,
       G_CALLBACK(glBindTexture),
       2,
       (guintptr) GL_TEXTURE_2D_ARRAY,
       (guintptr) tex->tios[i]);
    }
# if GL_ARB_direct_state_access == 1
  }
# endif // GL_ARB_direct_state_access
#endif // GL_VERSION_4_4
}
