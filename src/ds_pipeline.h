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
#ifndef __DS_PIPELINE_INCLUDED__
#define __DS_PIPELINE_INCLUDED__ 1
#include <ds_export.h>
#include <ds_renderable.h>
#include <ds_shader.h>
#include <gio/gio.h>

#define DS_TYPE_PIPELINE            (ds_pipeline_get_type())
#define DS_PIPELINE(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_PIPELINE, DsPipeline))
#define DS_PIPELINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_PIPELINE, DsPipelineClass))
#define DS_IS_PIPELINE(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_PIPELINE))
#define DS_IS_PIPELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_PIPELINE))
#define DS_PIPELINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_PIPELINE, DsPipelineClass))

typedef struct _DsPipeline      DsPipeline;
typedef struct _DsPipelineClass DsPipelineClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GQuark
ds_pipeline_error_quark();
DEUSEXMAKINA2_API
GType
ds_pipeline_get_type();

struct _DsPipelineClass
{
  GObjectClass parent_class;
};

DEUSEXMAKINA2_API
DsPipeline*
ds_pipeline_new(GCancellable   *cancellable,
                GError        **error);

DEUSEXMAKINA2_API
void
ds_pipeline_register_shader(DsPipeline   *pipeline,
                            const gchar  *shader_name,
                            int           priority,
                            DsShader     *shader);

DEUSEXMAKINA2_API
void
ds_pipeline_unregister_shader(DsPipeline   *pipeline,
                              const gchar  *shader_name);

DEUSEXMAKINA2_API
void
ds_pipeline_append_object(DsPipeline   *pipeline,
                          const gchar  *shader_name,
                          int           priority,
                          DsRenderable *object);

DEUSEXMAKINA2_API
void
ds_pipeline_remove_object(DsPipeline   *pipeline,
                          const gchar  *shader_name,
                          DsRenderable *object);

DEUSEXMAKINA2_API
gboolean
ds_pipeline_update(DsPipeline    *pipeline,
                   GCancellable  *cancellable,
                   GError       **error);

DEUSEXMAKINA2_API
void
ds_pipeline_execute(DsPipeline   *pipeline);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_PIPELINE_INCLUDED__
