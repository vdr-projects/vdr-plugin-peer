#include <vdr/status.h>
#include <vdr/thread.h>

#ifndef _PEER_NOTIFY__H
#define _PEER_NOTIFY__H

class cPeerNotify: public cStatus, public cThread {
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

#endif //_PEER_NOTIFY__H
