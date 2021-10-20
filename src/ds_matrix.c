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
#include <ds_matrix.h>

/**
 * SECTION:dsmatrix
 * @Short_description: Matrix placeholder
 * @Title: DsMatrix
 *
 * Holds necessary values to ensure proper alignment of matrices
 * even in case of unaligned dynamic allocations.
 *
 */

G_DEFINE_BOXED_TYPE
(DsMatrix,
 ds_matrix,
 ds_matrix_ref,
 ds_matrix_unref);

/**
 * ds_matrix_new: (constructor)
 * @n_floats: @floats array length. Must be 0, 2, 3, 4, 9 or 16.
 * @floats: (array length=n_floats) (nullable): initialization vector.
 *
 * Allocates a new instance of #DsMatrix.
 *
 * Returns: (transfer full): see description.
 */
DsMatrix*
ds_matrix_new(guint n_floats, gfloat floats[16])
{
  g_return_val_if_fail
  (   n_floats ==  0
   || n_floats ==  2
   || n_floats ==  3
   || n_floats ==  4
   || n_floats ==  9
   || n_floats == 16,
   NULL);
  if( n_floats != 0 )
    g_return_val_if_fail(floats != NULL, NULL);
  guint i;

  DsMatrix* matrix =
  g_slice_new(DsMatrix);
  g_atomic_ref_count_init(&(matrix->refs));

  guintptr offset = G_STRUCT_OFFSET(DsMatrix, _padding1_);
  guintptr address = (guintptr) matrix;
  guintptr rest = address % DS_MATRIX_ALIGNMENT;
  if G_LIKELY
    (rest != 0)
  {
    offset += DS_MATRIX_ALIGNMENT - rest;
  }

  matrix->value = G_STRUCT_MEMBER_P(matrix, offset);

  switch(n_floats)
  {
  case 16:
    matrix->value->full_[15] = floats[15];
    matrix->value->full_[14] = floats[14];
    matrix->value->full_[13] = floats[13];
    matrix->value->full_[12] = floats[12];
    matrix->value->full_[11] = floats[11];
    matrix->value->full_[10] = floats[10];
    matrix->value->full_[ 9] = floats[ 9];
    G_GNUC_FALLTHROUGH;
  case  9:
    matrix->value->full_[ 8] = floats[ 8];
    matrix->value->full_[ 7] = floats[ 7];
    matrix->value->full_[ 6] = floats[ 6];
    matrix->value->full_[ 5] = floats[ 5];
    matrix->value->full_[ 4] = floats[ 4];
    G_GNUC_FALLTHROUGH;
  case  4:
    matrix->value->full_[ 3] = floats[ 3];
    G_GNUC_FALLTHROUGH;
  case  3:
    matrix->value->full_[ 2] = floats[ 2];
    G_GNUC_FALLTHROUGH;
  case  2:
    matrix->value->full_[ 1] = floats[ 1];
    matrix->value->full_[ 0] = floats[ 0];
    break;
  }
return matrix;
}

/**
 * ds_matrix_ref: (method)
 * @matrix: an instance of #DsMatrix.
 *
 * Increments @matrix reference count.
 *
 * Returns: (transfer full): see description.
 */
DsMatrix*
ds_matrix_ref(DsMatrix* matrix)
{
  g_return_val_if_fail(matrix != NULL, NULL);
  g_atomic_ref_count_inc(&(matrix->refs));
return matrix;
}

/**
 * ds_matrix_unref: (method)
 * @matrix: an instance of #DsMatrix.
 *
 * Decrements @matrix reference count.
 * If reference count reaches zero @matrix
 * will be de-allocated.
 *
 */
void
ds_matrix_unref(DsMatrix* matrix)
{
  if G_LIKELY(matrix != NULL)
  {
    gboolean zero =
    g_atomic_ref_count_dec(&(matrix->refs));
    if(zero == TRUE)
    {
      g_slice_free(DsMatrix, matrix);
    }
  }
}

/**
 * ds_matrix_value: (method)
 * @matrix: an instance of #DsMatrix.
 *
 * Takes matrix->value pointer, useful for
 * language bindings.
 *
 * Returns: (transfer none): see description.
 */
DsMatrixValue*
ds_matrix_value(DsMatrix* matrix)
{
  g_return_val_if_fail(matrix != NULL, NULL);
return matrix->value;
}
