/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __EVENT_DISPATCHER__
#define __EVENT_DISPATCHER__

#include <map>
#include "cstdkbe/tasks.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/common.hpp"

namespace KBEngine { 
namespace Mercury
{

typedef TimesT<uint64> Times64;
class DispatcherCoupling;
class ErrorReporter;
class EventPoller;

class EventDispatcher
{
public:
	EventDispatcher();
	virtual ~EventDispatcher();
	
	void processContinuously();
	int  processOnce(bool shouldIdle = false);
	void processUntilBreak();

	void breakProcessing(bool breakState = true);
	bool processingBroken()const;
	
	void attach(EventDispatcher & childDispatcher);
	void detach(EventDispatcher & childDispatcher);
	
	void addFrequentTask(Task * pTask);
	bool cancelFrequentTask(Task * pTask);
	
	INLINE double maxWait() const;
	INLINE void maxWait(double seconds);

	bool registerFileDescriptor(int fd, InputNotificationHandler * handler);
	bool deregisterFileDescriptor(int fd);
	bool registerWriteFileDescriptor(int fd, InputNotificationHandler * handler);
	bool deregisterWriteFileDescriptor(int fd);

	INLINE TimerHandle addTimer(int64 microseconds,
					TimerHandler * handler, void* arg = NULL);
	INLINE TimerHandle addOnceOffTimer(int64 microseconds,
					TimerHandler * handler, void * arg = NULL);

	uint64 timerDeliveryTime(TimerHandle handle) const;
	uint64 timerIntervalTime(TimerHandle handle) const;
	uint64 & timerIntervalTime(TimerHandle handle);
	
	uint64 getSpareTime() const;
	void clearSpareTime();
	double proportionalSpareTime() const;

	ErrorReporter & errorReporter()	{ return *m_pErrorReporter_; }

	INLINE EventPoller* createPoller();
private:
	TimerHandle addTimerCommon(int64 microseconds,
		TimerHandler * handler,
		void * arg,
		bool recurrent);

	void processFrequentTasks();
	void processTimers();
	void processStats();
	int processNetwork(bool shouldIdle);
	
	double calculateWait() const;
	
	void attachTo(EventDispatcher & parentDispatcher);
	void detachFrom(EventDispatcher & parentDispatcher);
	
	typedef std::vector<EventDispatcher *> ChildDispatchers;
	ChildDispatchers childDispatchers_;

protected:
	bool m_breakProcessing_;
	double m_maxWait_;
	uint32 m_numTimerCalls_;
	
	// Statistics
	TimeStamp		m_accSpareTime_;
	TimeStamp		m_oldSpareTime_;
	TimeStamp		m_totSpareTime_;
	TimeStamp		m_lastStatisticsGathered_;
	
	Tasks* m_pFrequentTasks_;
	ErrorReporter * m_pErrorReporter_;
	Times64* m_pTimes_;
	EventPoller* m_pPoller_;
	DispatcherCoupling * m_pCouplingToParent_;
};


class DispatcherCoupling : public Task
{
public:
	DispatcherCoupling(EventDispatcher & mainDispatcher,
			EventDispatcher & childDispatcher) :
		mainDispatcher_(mainDispatcher),
		childDispatcher_(childDispatcher)
	{
		mainDispatcher.addFrequentTask(this);
	}

	~DispatcherCoupling()
	{
		mainDispatcher_.cancelFrequentTask(this);
	}

private:
	void process()
	{
		childDispatcher_.processOnce();
	}

	EventDispatcher & mainDispatcher_;
	EventDispatcher & childDispatcher_;
};

}
}

#ifdef CODE_INLINE
#include "event_dispatcher.ipp"
#endif
#endif // __EVENT_DISPATCHER__