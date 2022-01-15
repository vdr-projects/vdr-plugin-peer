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

#include "status.h"
#include "svdrp.h"

class cPeerNotify: public cListObject, public cThread {
private:
	cString peer;
	enum { nsInit, nsWaiting, nsFinished } notifyState;
protected:
	void Action();
public:
	bool Finished() const { return notifyState == nsFinished; }
	cPeerNotify(const char* Peer);
	virtual ~cPeerNotify();
};

cPeerNotify::cPeerNotify(const char *Peer): peer(Peer) {
	notifyState = nsInit;
	Start();
}

cPeerNotify::~cPeerNotify() {
}

void cPeerNotify::Action() {
	const char *msg = notifyState == nsInit ? tr("Notification scheduled") : tr("Replay ended");
	int retries = 30;
	while (--retries > 0) {
		if (cSvdrp::GetInstance()->CmdMESG(peer, msg)) {
			notifyState = notifyState == nsInit ? nsWaiting : nsFinished;
			return;
		}
		else if (Running())
			cCondWait::SleepMs(1000);
		else
			retries = 0;
	}
	esyslog("peer: failed to notify %s", *peer);
	notifyState = nsFinished;
}

cPeerStatusRecording::cPeerStatusRecording(const cDevice *Device, const char *Name, const char *FileName):
	device(Device), name(Name), fileName(FileName)
{}

void cPeerStatus::Recording(const cDevice *Device, const char *Name, const char *FileName, bool On)
{
	if (On) {
		recording.Add(new cPeerStatusRecording(Device, Name, FileName));
	}
	else {
		for (cPeerStatusRecording *rec = recording.First(); rec; recording.Next(rec)) {
			if (Device == rec->device && strcmp(FileName, *rec->fileName) == 0) {
				recording.Del(rec);
				break;
			}
		}
	}
}

void cPeerStatus::Replaying(const cControl *Control, const char *Name, const char *FileName, bool On)
{
	if (On)
		replayName = Name;
	else {
		for (cPeerNotify *n = replayNotifies.First(); n; n = replayNotifies.Next(n))
			n->Finished() || n->Start();
		replayName = NULL;
	}
}

void cPeerStatus::AddReplayNotify(const char *Peer)
{
	cPeerNotify *del = NULL;
	for (cPeerNotify *n = replayNotifies.First(); n; n = replayNotifies.Next(n)) {
		if (del) {
			replayNotifies.Del(del);
			del = NULL;
		}
		if (n->Finished())
			del = n;
	}
	replayNotifies.Add(new cPeerNotify(Peer));
}
