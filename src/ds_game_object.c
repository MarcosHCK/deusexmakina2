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
#include <ds_game_object.h>
#include <ds_macros.h>
#include <ds_model.h>
#include <ds_mvpholder.h>
#include <ds_renderable.h>
#include <jit/jit.h>

/**
 * SECTION:dsgameobject
 * @Short_description: A game object
 * @Title: DsGameObject
 *
 * DsGameObject is the basic foundation of all objects which exists
 * within deusexmakina2.
 *
 */

static void
ds_game_object_ds_renderable_iface_init(DsRenderableIface* iface);
static void
ds_game_object_ds_mvp_holder_iface_init(DsMvpHolderIface* iface);

/*
 * Object definition
 *
 */

struct _DsGameObjectPrivate
{
  vec3 scale;
  vec3 position;
  mat4 model;

  union
  {
    DsModel* model_;
    DsRenderable* draw;
  };
};

enum {
  prop_0,
  prop_model,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsGameObject,
 ds_game_object,
 G_TYPE_OBJECT,
 G_ADD_PRIVATE(DsGameObject)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_RENDERABLE,
  ds_game_object_ds_renderable_iface_init)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_MVP_HOLDER,
  ds_game_object_ds_mvp_holder_iface_init));

static gboolean
ds_game_object_ds_renderable_iface_compile(DsRenderable* pself, DsRenderState* state, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsGameObjectPrivate* priv =
  ((DsGameObject*) pself)->priv;

  GLuint program;
  GLuint uloc;

  program = ds_render_state_get_current_program(state);

/*
 * Update model matrix
 *
 */

  __gl_try_catch(
    uloc = glGetUniformLocation(program, A_MVP);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_LIKELY(uloc != (-1))
  {
    JitState* ctx =
    (JitState*) state;

    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(_ds_jit_helper_update_model),
     FALSE,
     2,
     (guintptr) &(ctx->mvps),
     (guintptr) priv->model);

    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(_ds_jit_helper_update_mvp),
     FALSE,
     1,
     (guintptr) &(ctx->mvps));

    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(glUniformMatrix4fv),
     TRUE,
     4,
     (guintptr) uloc,
     (guintptr) 1,
     (guintptr) GL_FALSE,
     (guintptr) &(ctx->mvps.mvp));
  }

/*
 * Draw model
 *
 */

  success =
  ds_renderable_compile(priv->draw, state, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
return success;
}

static void
ds_game_object_ds_renderable_iface_init(DsRenderableIface* iface)
{
  iface->compile = ds_game_object_ds_renderable_iface_compile;
}

static void
update_model(DsGameObjectPrivate* priv)
{
  glm_mat4_identity(priv->model);
  glm_translate(priv->model, priv->position);
  glm_scale(priv->model, priv->scale);
}

static void
ds_game_object_ds_mvp_holder_iface_set_position(DsMvpHolder* pself, gfloat* src)
{
  DsGameObjectPrivate* priv = ((DsGameObject*)pself)->priv;
  glm_vec3_copy(src, priv->position);
  update_model(priv);
}

static void
ds_game_object_ds_mvp_holder_iface_get_position(DsMvpHolder* pself, gfloat* dst)
{
  DsGameObjectPrivate* priv = ((DsGameObject*)pself)->priv;
  glm_vec3_copy(priv->position, dst);
}

static void
ds_game_object_ds_mvp_holder_iface_set_scale(DsMvpHolder* pself, gfloat* src)
{
  DsGameObjectPrivate* priv = ((DsGameObject*)pself)->priv;
  glm_vec3_copy(src, priv->scale);
  update_model(priv);
}

static void
ds_game_object_ds_mvp_holder_iface_get_scale(DsMvpHolder* pself, gfloat* dst)
{
  DsGameObjectPrivate* priv = ((DsGameObject*)pself)->priv;
  glm_vec3_copy(priv->scale, dst);
}

static void
ds_game_object_ds_mvp_holder_iface_init(DsMvpHolderIface* iface)
{
  iface->p_model = G_PRIVATE_OFFSET(DsGameObject, model);
  iface->set_position = ds_game_object_ds_mvp_holder_iface_set_position;
  iface->get_position = ds_game_object_ds_mvp_holder_iface_get_position;
  iface->set_scale = ds_game_object_ds_mvp_holder_iface_set_scale;
  iface->get_scale = ds_game_object_ds_mvp_holder_iface_get_scale;
}

static void
ds_game_object_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsGameObjectPrivate* priv = DS_GAME_OBJECT(pself)->priv;
  switch(prop_id)
  {
  case prop_model:
    g_set_object(&(priv->model_), g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static void
ds_game_object_class_get_property(GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec)
{
  DsGameObjectPrivate* priv = DS_GAME_OBJECT(pself)->priv;
  switch(prop_id)
  {
  case prop_model:
    g_value_set_object(value, priv->model_);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static void
ds_game_object_class_finalize(GObject* pself)
{
  DsGameObjectPrivate* priv = DS_GAME_OBJECT(pself)->priv;
G_OBJECT_CLASS(ds_game_object_parent_class)->finalize(pself);
}

static void
ds_game_object_class_dispose(GObject* pself)
{
  DsGameObjectPrivate* priv = DS_GAME_OBJECT(pself)->priv;
  g_clear_object(&(priv->model_));
G_OBJECT_CLASS(ds_game_object_parent_class)->dispose(pself);
}

static void
ds_game_object_class_init(DsGameObjectClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  oclass->set_property = ds_game_object_class_set_property;
  oclass->get_property = ds_game_object_class_get_property;
  oclass->finalize = ds_game_object_class_finalize;
  oclass->dispose = ds_game_object_class_dispose;

  properties[prop_model] =
    g_param_spec_object
    ("model",
     "Model object",
     "Model object",
     DS_TYPE_MODEL,
     G_PARAM_READWRITE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static void
ds_game_object_init(DsGameObject* self)
{
  DsGameObjectPrivate* priv = ds_game_object_get_instance_private(self);
  self->priv = priv;

  glm_mat4_identity(priv->model);
}

/*
 * Methods
 *
 */

