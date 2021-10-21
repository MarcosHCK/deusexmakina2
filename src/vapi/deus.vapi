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

namespace Ds
{
  [CCode (cheader_filename = "ds_mvpholder.h")]
  public interface MvpHolder
  {
    /* notifiers */
    public virtual void notify_model();
    public virtual void notify_view();
    public virtual void notify_projection();

    /* abstractions */
    public virtual void set_position(float position[3]);
    public virtual void get_position(float position[3]);
    public virtual void set_scale(float scale[3]);
    public virtual void get_scale(float scale[3]);
  }
}
