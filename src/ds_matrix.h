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
#ifndef __DS_MATRIX_INCLUDED__
#define __DS_MATRIX_INCLUDED__
#include <cglm/cglm.h>
#include <ds_export.h>
#include <glib-object.h>

typedef struct _DsMatrix      DsMatrix;
typedef union  _DsMatrixValue DsMatrixValue;

#define DS_MATRIX_ALIGNMENT (G_ALIGNOF (mat4))

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_matrix_get_type();

/**
 * DsMatrixValue:
 * @vec2_: (array fixed-size=2) (element-type float): 2-dimensions array.
 * @vec3_: (array fixed-size=3) (element-type float): 3-dimensions array.
 * @vec4_: (array fixed-size=4) (element-type float): 4-dimensions array.
 * @mat2_: (array fixed-size=4) (element-type float): 2 by 2 dimensions matrix.
 * @mat3_: (array fixed-size=9) (element-type float): 3 by 3 dimensions matrix.
 * @mat4_: (array fixed-size=16) (element-type float): 4 by 4 dimensions matrix.
 *
 * Holds any kind of matrices and vectors in a packed and aligned structure
 *
 */
CGLM_ALIGN_MAT
union _DsMatrixValue
{
  float scalar;
  vec2 vec2_;
  vec3 vec3_;
  vec4 vec4_;
  mat2 mat2_;
  mat3 mat3_;
  mat4 mat4_;

  /*<private>*/
  float full_[16];
};

/**
 * DsMatrix:
 * @value: matrices value (a pointer to an internal #DsMatrixValue instance).
 *
 * Holds necessary values to ensure proper alignment of matrices
 * even in case of unaligned dynamic allocations.
 *
 */
struct _DsMatrix
{
  /*<protected>*/
  gatomicrefcount refs;
  /*<public>*/
  DsMatrixValue* value;

  /*<private>*/
#ifndef __GI_SCANNER__
  gchar _padding1_[DS_MATRIX_ALIGNMENT];
  DsMatrixValue _padding2_;
#endif // __GI_SCANNER__
};

DEUSEXMAKINA2_API
DsMatrix*
ds_matrix_new(guint n_floats, gfloat* floats);
DEUSEXMAKINA2_API
DsMatrix*
ds_matrix_ref(DsMatrix* matrix);
DEUSEXMAKINA2_API
void
ds_matrix_unref(DsMatrix* matrix);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_MATRIX_INCLUDED__
