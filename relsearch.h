/* This file is part of Ariko.
 *
 * Ariko is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ariko is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ariko.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RELSEARCH_H_INCLUDED
#define RELSEARCH_H_INCLUDED

#include <stdbool.h>

int* relSearch8Bit(char*, char*);
int* relSearch16Bit(char*, char*, bool);
int* relSearch(char*, char*, int, bool);

#endif // RELSEARCH_H_INCLUDED
