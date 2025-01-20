#include "pch.h"
#include "Timer.h"
#include"Iocp.h"


Timer::Timer()
{
}
Timer::~Timer()
{
}
void Timer::TimerThread()
{
	while (isRunning)
	{
		TimerEvent ev;
		if (true == _timerqueue.try_pop(ev))
		{
		auto cur_time = std::chrono::system_clock::now();

			if (ev.wakeupTime > cur_time)
			{
				_timerqueue.push(ev);
				this_thread::yield();
				continue;
			}
			switch (ev.evtype)
			{
			case EVENT_TYPE::EV_INIT:
			{
				auto& instance = Iocp::GetInstance();
				Over* ov = new Over;
				ov->_type = CompType::NpcInit;
				ov->_id = ev.n_id;

				if (PostQueuedCompletionStatus(_iocpHandle, 1, ev.c_id, &ov->_over) == FALSE)
				{
					std::cerr << "Failed to post to IOCP: " << GetLastError() << std::endl;
				}
				break;
			}
			case EVENT_TYPE::EV_NPC_MOVE:
			{
				auto& instance = Iocp::GetInstance();
				Over* ov = new Over;
				ov->_type = CompType::NpcMove;
				ov->_id = ev.n_id;

				if (PostQueuedCompletionStatus(_iocpHandle, 1, ev.c_id, &ov->_over) == FALSE)
				{
					std::cerr << "Failed to post to IOCP: " << GetLastError() << std::endl;
				}
				break;
			}
			}
		}continue;

	}
}

void Timer::InitTimerQueue(TimerEvent ev)
{
	lock_guard<mutex> timerlockguard{ _TimerQueueLock };
	_timerqueue.push(ev);
}

void Timer::Run()
{
	isRunning = true;
	_timerthread = std::thread{ [&]() {TimerThread(); } };
}

void Timer::End()
{
	isRunning = false;
	if (_timerthread.joinable())
		_timerthread.join();
}
