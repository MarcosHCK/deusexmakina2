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
#include <ds_application.h>
#include <ds_macros.h>
#include <girepository.h>
#include <luad_core.h>
#undef main

static void
_lua_close0(lua_State* var)
{
  (var == NULL) ? NULL : (var = (luaD_close (var), NULL));
}

/*
 * Option callbacks
 *
 */

static gboolean
version_arg(const gchar  *option_name,
            const gchar  *value,
            gpointer      data,
            GError      **error)
{
  g_print(PACKAGE_STRING "\r\n");
return FALSE;
}

/*
 * main()!
 *
 */

int
main(int argc, char* argv[])
{
/*
 * Create application
 *
 */

  GError* tmp_err = NULL;
  GApplication* app = NULL;
  int return_ = 0;

  /*
   * Leave initialization for now
   * because it needs some data
   * passed through command line
   *
   */
  app = (GApplication*)
  g_object_new
  (DS_TYPE_APPLICATION,
   "application-id", GAPPNAME,
   "flags", G_APPLICATION_FLAGS_NONE,
   NULL);

  static const
  GOptionEntry entries[] =
  {
    {"version", 'v', G_OPTION_FLAG_IN_MAIN | G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, version_arg, "Displays version information", NULL},
    {NULL, '\0', 0, 0, NULL, NULL, NULL},
  };

  g_application_add_main_option_entries(app, entries);

/*
 * Run main loop
 *
 */

  return_ =
  g_application_run(app, argc, argv);

/*
 * Finalize application object
 *
 */

  DsApplication* self = DS_APPLICATION(app);
  lua_State* L = (lua_State*) self->lua;

  /* Collects all unused references, thus it may destroy some objects */
  if G_LIKELY(self->lua != NULL)
    lua_gc(L, LUA_GCCOLLECT, 1);

  g_clear_pointer(&(self->lua), _lua_close0);   /* Release a possibly held reference to application object          */
#if DEVELOPER == 1
  g_assert_finalize_object(app);              /* Finally destroy application object                               */
#else
  g_object_unref(app);
#endif // DEVELOPER
return return_;
}
