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
#include <ds_event_types.h>
#include <ds_events.h>
#include <ds_looper.h>
#include <ds_macros.h>
#include <ds_marshals.h>
#include <GLFW/glfw3.h>
#include <luad_core.h>

G_DEFINE_QUARK(ds-window-events-quark,
               ds_window_events);

G_DEFINE_QUARK(motion,
               ds_cursor_motion);
G_DEFINE_QUARK(button,
               ds_cursor_button);
G_DEFINE_QUARK(scroll,
               ds_cursor_scroll);
G_DEFINE_QUARK(char,
               ds_keyboard_unichar);
G_DEFINE_QUARK(key,
               ds_keyboard_key);

/**
 * SECTION:dsevents
 * @Short_description: events controller
 * @Title: DsEvents
 *
 * DsEvents encapsulates complexities of
 * launching events into Lua space
 *
 */

#define EVENT_PUSH "__DS_EVENT_PUSH"

static void
ds_events_g_initable_iface_init(GInitableIface* iface);

enum
{
  conn_invert_y,
  conn_number,
};

static void
on_window_close(GLFWwindow* window);
static void
on_cursor_motion(GLFWwindow* window, double x, double y);
static void
on_cursor_button(GLFWwindow* window, int button, int action, int mods);
static void
on_cursor_scroll(GLFWwindow* window, double x_offs, double y_offs);
static void
on_keyboard_unichar(GLFWwindow* window, gunichar codepoint);
static void
on_keyboard_key(GLFWwindow* window, int key, int scancode, int action, int mods);

/*
 * Object definition
 *
 */

struct _DsEvents
{
  DsLooper parent_instance;

  /*<private>*/
  GSettings* gsettings;
  GLFWwindow* window;

  double x_prev, y_prev;
  double x_fact, y_fact;

  gulong connections[conn_number];
};

struct _DsEventsClass
{
  DsLooperClass parent_class;
};

enum
{
  prop_0,
  prop_gsettings,
  prop_window,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

enum
{
  sig_cursor,
  sig_keyboard,
  sig_number,
};

static
guint signals[sig_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsEvents,
 ds_events,
 DS_TYPE_LOOPER,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_events_g_initable_iface_init));

static void
on_invert_y_changed(GSettings       *gsettings,
                    const gchar     *key,
                    DsEvents        *self)
{
  gboolean invert_y;
  g_settings_get(gsettings, key, "b", &invert_y);
  self->y_fact = (invert_y) ? -1.d : 1.d;
}


static gboolean
ds_events_g_initable_iface_init_sync(GInitable* pself, GCancellable* cancellable, GError** error)
{
  DsEvents* self = DS_EVENTS(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * Add ourself to window's datalist
 *
 */

  GData** datalist =
  glfwGetWindowUserPointer(self->window);

  g_datalist_id_set_data_full
  (datalist,
   ds_window_events_quark(),
   g_object_ref(self),
   g_object_unref);

/*
 * Register callbacks
 *
 */

  glfwSetWindowCloseCallback(self->window, on_window_close);
  glfwSetCursorPosCallback(self->window, on_cursor_motion);
  glfwSetMouseButtonCallback(self->window, on_cursor_button);
  glfwSetScrollCallback(self->window, on_cursor_scroll);
  glfwSetCharCallback(self->window, on_keyboard_unichar);
  glfwSetKeyCallback(self->window, on_keyboard_key);

/*
 * Connects settings
 *
 */

  self->x_fact = 1.f;
  self->y_fact = 1.f;

  self->connections[conn_invert_y] =
  g_signal_connect
  (self->gsettings,
   "changed::invert-y",
   G_CALLBACK(on_invert_y_changed),
   self);
  on_invert_y_changed(self->gsettings, "invert-y", self);

_error_:
return success;
}

static void
ds_events_g_initable_iface_init(GInitableIface* iface)
{
  iface->init = ds_events_g_initable_iface_init_sync;
}

static gboolean
ds_events_class_loop_step(DsLooper* pself)
{
  glfwPollEvents();
return G_SOURCE_CONTINUE;
}

static void
ds_events_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsEvents* self = DS_EVENTS(pself);
  switch(prop_id)
  {
  case prop_gsettings:
    g_set_object(&(self->gsettings), g_value_get_object(value));
    break;
  case prop_window:
    self->window = g_value_get_pointer(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static void
ds_events_class_dispose(GObject* pself)
{
  DsEvents* self = DS_EVENTS(pself);
  guint i;

  for(i = 0;
      i < conn_number;
      i++)
  if(self->connections[i] != 0)
  {
    g_signal_handler_disconnect(self->gsettings, self->connections[i]);
    self->connections[i] = 0;
  }

  g_clear_object(&(self->gsettings));
G_OBJECT_CLASS(ds_events_parent_class)->dispose(pself);
}

static void
ds_events_class_init(DsEventsClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);
  DsLooperClass* lclass = DS_LOOPER_CLASS(klass);

  lclass->loop_step = ds_events_class_loop_step;

  oclass->set_property = ds_events_class_set_property;
  oclass->dispose = ds_events_class_dispose;

  /**
   * DsEvents::cursor:
   * @object: the #DsEvent object which launches the signal.
   * @detail: event type name.
   * @event: (type Ds.EvCursor): event descriptor.
   *
   * Launches a generic 'cursor' namespace event.
   *
   */
  signals[sig_cursor] =
    g_signal_new
    ("cursor",
     G_TYPE_FROM_CLASS(klass),
     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
     0,
     NULL,
     NULL,
     ds_cclosure_marshal_VOID__STRING_POINTER,
     G_TYPE_NONE,
     2,
     G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
     G_TYPE_POINTER | G_SIGNAL_TYPE_STATIC_SCOPE,
     G_TYPE_NONE);

  /**
   * DsEvents::keyboard:
   * @object: the #DsEvent object which launches the signal.
   * @detail: event type name.
   * @event: (type Ds.EvKeyboard): event descriptor.
   *
   * Launches a generic 'keyboard' namespace event.
   *
   */
  signals[sig_keyboard] =
    g_signal_new
    ("keyboard",
     G_TYPE_FROM_CLASS(klass),
     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
     0,
     NULL,
     NULL,
     ds_cclosure_marshal_VOID__STRING_POINTER,
     G_TYPE_NONE,
     2,
     G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
     G_TYPE_POINTER | G_SIGNAL_TYPE_STATIC_SCOPE,
     G_TYPE_NONE);

  g_signal_set_va_marshaller
  (signals[sig_cursor],
   G_TYPE_FROM_CLASS(klass),
   ds_cclosure_marshal_VOID__STRING_POINTERv);

  g_signal_set_va_marshaller
  (signals[sig_keyboard],
   G_TYPE_FROM_CLASS(klass),
   ds_cclosure_marshal_VOID__STRING_POINTERv);

  properties[prop_gsettings] =
    g_param_spec_object
    (_TRIPLET("gsettings"),
     G_TYPE_SETTINGS,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_window] =
    g_param_spec_pointer
    (_TRIPLET("window"),
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static void
ds_events_init(DsEvents* self)
{
}

/*
 * Support
 *
 */

static inline DsEvents*
this_(GLFWwindow* window)
{
  GData** datalist =
  glfwGetWindowUserPointer(window);

  DsEvents* self =
  g_datalist_id_get_data
  (datalist,
   ds_window_events_quark());
return self;
}

static inline void
push_(lua_State* L, int argc)
{
  GError* tmp_err = NULL;
  int top = lua_gettop(L);
  int func = top - argc + 1;

  lua_getfield(L, LUA_REGISTRYINDEX, EVENT_PUSH);
  lua_insert(L, func);

  luaD_xpcall(L, argc, 0, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_critical
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free(tmp_err);
    g_assert_not_reached();
  }
}

/*
 * Event callbacks
 *
 */

#define EMIT(object,namespace,detail,event) \
  G_STMT_START { \
    GValue values[3] = {0}; \
    GQuark quark = ds_##namespace##_##detail##_quark(); \
    g_value_init(values + 0, DS_TYPE_EVENTS); \
    g_value_set_object(values + 0, (object)); \
    g_value_init(values + 1, G_TYPE_STRING); \
    g_value_set_static_string(values + 1, g_quark_to_string(quark)); \
    g_value_init(values + 2, G_TYPE_POINTER); \
    g_value_set_pointer(values + 2, (event)); \
    g_signal_emitv(values, signals[ sig_##namespace ], quark, NULL); \
    g_value_unset(values + 2); \
    g_value_unset(values + 1); \
    g_value_unset(values + 0); \
  } G_STMT_END;

static void
on_window_close(GLFWwindow* window)
{
  g_application_quit(g_application_get_default());
}

static void
on_cursor_motion(GLFWwindow* window, double x, double y)
{
  DsEvents* self = this_(window);
  DsEvCursorMotion value;

  value.x = x;
  value.y = y;
  value.dx = (x - self->x_prev) * self->x_fact;
  value.dy = (y - self->y_prev) * self->y_fact;
  self->x_prev = x;
  self->y_prev = y;
EMIT(self, cursor, motion, &value);
}

static void
on_cursor_button(GLFWwindow* window, int button, int action, int mods)
{
  DsEvents* self = this_(window);
  DsEvCursorButton value;

  value.button = button;
  value.action = action;
  value.mods = mods;
EMIT(self, cursor, button, &value);
}

static void
on_cursor_scroll(GLFWwindow* window, double x_offs, double y_offs)
{
  DsEvents* self = this_(window);
  DsEvCursorScroll value;

  value.dx = x_offs;
  value.dy = y_offs;
EMIT(self, cursor, scroll, &value);
}

static void
on_keyboard_unichar(GLFWwindow* window, gunichar codepoint)
{
  DsEvents* self = this_(window);
  DsEvKeyboardUnichar value;

  value.codepoint = codepoint;
EMIT(self, keyboard, unichar, &value);
}

static void
on_keyboard_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  DsEvents* self = this_(window);
  DsEvKeyboardKey value;

  value.key = key;
  value.scancode = scancode;
  value.action = action;
  value.mods = mods;
EMIT(self, keyboard, key, &value);
}

#undef EMIT

/*
 * Object methods
 *
 */

/**
 * ds_events_new: (constructor) (skip)
 * @gsettings: (not nullable): a #GSettings object.
 * @window: (not nullable): SDL window object.
 * @cancellable: (nullable): a %GCancellable.
 * @error: return location for a #GError.
 *
 * Creates a new #DsEvents which operates on
 * @engine Lua state.
 *
 * Returns: (transfer full): see description.
 */
DsEvents*
ds_events_new(GSettings      *gsettings,
              gpointer        window,
              GCancellable   *cancellable,
              GError        **error)
{
  return (DsEvents*)
  g_initable_new
  (DS_TYPE_EVENTS,
   cancellable,
   error,
   "gsettings", gsettings,
   "window", window,
   NULL);
}
