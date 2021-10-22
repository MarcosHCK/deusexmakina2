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
#ifndef __DS_EVENT_TYPES_INCLUDED__
#define __DS_EVENT_TYPES_INCLUDED__
#include <glib.h>

typedef struct _DsEvCursorMotion    DsEvCursorMotion;
typedef struct _DsEvCursorButton    DsEvCursorButton;
typedef struct _DsEvCursorScroll    DsEvCursorScroll;
typedef union  _DsEvCursor          DsEvCursor;
typedef struct _DsEvKeyboardUnichar DsEvKeyboardUnichar;
typedef struct _DsEvKeyboardKey     DsEvKeyboardKey;
typedef union  _DsEvKeyboard        DsEvKeyboard;

/**
 * DsEvCursorMotion:
 * @x: cursor's position on X-axis.
 * @y: cursor's position on Y-axis.
 * @dx: cursor's movement vector value on X-axis.
 * @dy: cursor's movement vector value on Y-axis.
 *
 * Represents a cursor motion event.
 *
 */
struct _DsEvCursorMotion
{
  gdouble x;
  gdouble y;
  gdouble dx;
  gdouble dy;
};

/**
 * DsEvCursorButton:
 * @button: cursor's button.
 * @action: button event type.
 * @mods: modifications.
 *
 * Represents a cursor button event.
 *
 */
struct _DsEvCursorButton
{
  gint button;
  gint action;
  guint mods;
};

/**
 * DsEvCursorScroll:
 * @dx: cursor's horizontal wheel movement direction (positive means up).
 * @dy: cursor's vertical wheel movement direction (positive means up).
 *
 * Represents a cursor scroll event.
 *
 */
struct _DsEvCursorScroll
{
  gdouble dx;
  gdouble dy;
};

/**
 * DsEvCursor:
 * @motion: motion event.
 * @button: button event.
 * @scroll: scroll event.
 *
 * Represents every posible events on 'cursor' namespace.
 *
 */
union _DsEvCursor
{
  DsEvCursorMotion motion;
  DsEvCursorButton button;
  DsEvCursorScroll scroll;
};

/**
 * DsEvKeyboardUnichar:
 * @codepoint: unicode codepoint.
 *
 * Represents an unicode symbol inputed from keyboard.
 *
 */
struct _DsEvKeyboardUnichar
{
  gunichar codepoint;
};

/**
 * DsEvKeyboardKey:
 * @key: keyboard layout independent key code.
 * @scancode: keyboard dependent key code (useful for not standard keys).
 * @action: key event type.
 * @mods: modifications.
 *
 * Represents a key being pressed, held or released on keyboard.
 *
 */
struct _DsEvKeyboardKey
{
  gint key;
  gint scancode;
  gint action;
  guint mods;
};

/**
 * DsEvKeyboard:
 * @unichar: unichar event.
 * @key: key event.
 *
 * Represents every posible events on 'keyboard' namespace.
 *
 */
union _DsEvKeyboard
{
  DsEvKeyboardUnichar unichar;
  DsEvKeyboardKey key;
};

#endif // __DS_EVENT_TYPES_INCLUDED__
