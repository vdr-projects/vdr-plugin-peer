/*
 * Copyright (C) 2013 Frank Schmirler <vdr@schmirler.de>
 *
 * This file is part of VDR Plugin peer.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

#ifndef _PEER_TOOLS__H
#define _PEER_TOOLS__H

#include <vdr/tools.h>

#define STRINGBUFFER_DEFAULT_SIZE 128

class cStringBuffer
{
private:
	int length;
	int size;
	char *buffer;
public:
	cStringBuffer& Append(const char *Fmt, ...) __attribute__ ((format (printf, 2, 3)));
	// take ownership of the current buffer; NULL if out of memory
	char* Export();
	// current string length; -1 if out of memory
	int Length() const { return buffer ? length : -1; }

	cStringBuffer(int Size = STRINGBUFFER_DEFAULT_SIZE);
	~cStringBuffer();
};

bool Parse(char *s, cString& Key, cString& Value);
bool Parse(const char *s, cString& Key, cString& Value);
void ParseIpPort(const char *s, cString& Ip, unsigned short& Port);

bool WakeOnLAN(const char *Mac);

#endif // _PEER_TOOLS__H
