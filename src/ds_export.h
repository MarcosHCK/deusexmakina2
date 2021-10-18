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
#ifndef __DS_EXPORT_INCLUDED__
#define __DS_EXPORT_INCLUDED__ 1

#if __DUESEXMAKINA_INSIDE__ && __GNUC__ >= 4
# define DEUSEXMAKINA2_API __attribute__((__visibility__("default")))
#elif __DUESEXMAKINA_INSIDE__ && defined _MSC_VER
# define DEUSEXMAKINA2_API __declspec(dllexport)
#elif defined _MSC_VER
# define DEUSEXMAKINA2_API __declspec(dllimport)
#else
# define DEUSEXMAKINA2_API
#endif

#endif // __DS_EXPORT_INCLUDED__
