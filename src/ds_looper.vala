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
  public abstract class Looper : GLib.Object
  {
    private uint source_id = 0;
    private GLib.Source source = null;
    protected abstract bool loop_step();

    public void start()
    {
      if(likely(source_id == 0))
      {
        source_id = source.attach(null);
      }
    }

    public void stop()
    {
      if(likely(source_id != 0))
      {
        GLib.Source.remove(source_id);
        source_id = 0;
      }
    }

    construct
    {
      source = new GLib.IdleSource();
      source.set_priority(GLib.Priority.DEFAULT);
      source.set_name(@"(Source) for type '$(GLib.Type.from_instance(this).name())'");

      unowned var callback = (GLib.SourceFunc) this.loop_step;
      source.set_callback(callback);
    }

    ~Looper()
    {
      this.stop();
    }
  }
}
