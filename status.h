#include <vdr/status.h>
#include <vdr/tools.h>

#ifndef _PEER_STATUS__H
#define _PEER_STATUS__H

class cPeerNotify;

class cPeerStatusRecording : public cListObject {
	friend class cPeerStatus;
private:
	const cDevice *device;
	cString       name;
	cString       fileName;
	cPeerStatusRecording(const cDevice *Device, const char *Name, const char *FileName);
public:
	const cDevice* Device() const { return device; }
	const char* Name() const { return *name; }
};

class cPeerStatus : public cStatus {
private:
	cString                     replayName;
	cList<cPeerNotify>          replayNotifies;
	cList<cPeerStatusRecording> recording;
protected:
	virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
	virtual void Replaying(const cControl *Control, const char *Name, const char *FileName, bool On);
public:
	const cList<cPeerStatusRecording>* Recording() const { return &recording; }
	const char* Replaying() const { return *replayName; }
	void AddReplayNotify(const char *Peer);
};

#endif //_PEER_STATUS__H
