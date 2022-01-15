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

#include "notify.h"

cPeerNotify::cPeerNotify(const char *Peer): peer(Peer) {
	notifyState = nsInit;
	Start();
}

cPeerNotify::~cPeerNotify() {
}

void cPeerNotify::Action() {
	cSvdrp *svdrp = cSvdrp::GetInstance();
	const char *msg = nsInit ? tr("Notification scheduled") : tr("Replay ended");
	int retries = 30;
	while (--retries > 0) {
		if (svdrp->CmdMESG(Peer, msg)) {
			notifyState = nsInit ? nsWaiting : nsFinished;
			return;
		}
		else if (Running())
			cCondWait::SleepMs(1000);
		else
			retries = 0;
	}
	notifyState = nsFinished;
}

