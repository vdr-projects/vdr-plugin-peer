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

#ifndef _PEER_SVDRP__H
#define _PEER_SVDRP__H

#include "include/svdrpservice.h"
#include <vdr/plugin.h>

class cOsdMenu;

class cSvdrp {
	private:
		static cSvdrp        *svdrp;

		SvdrpConnection_v1_0 conn;
		SvdrpCommand_v1_0    cmd;
		cPlugin              *service;

		unsigned short Send(const char *fmt, ...);
	public:
		static cSvdrp* GetInstance();
		static void DeleteInstance();

		// Methods to be used in main thread only
		bool Connect(const char *ServerIp, unsigned short ServerPort);
		bool CmdMESG(const char *Message);
		bool CmdPEER_NTFY(const char *IpPort);
		const cList<cLine> *CmdPEER_STAT();
		//unsigned short CmdPEER_CONF();
		void Disconnect();

		// Threadsafe methods
		bool CmdMESG(const char *Peer, const char *Message);

		cSvdrp(cPlugin *Service);
		~cSvdrp();
};

#endif //_PEER_SVDRP__H
