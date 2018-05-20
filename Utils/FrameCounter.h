#pragma once

#include <functional>
#include <forward_list>

typedef std::function<void(size_t)> fps_notification_t;

class CFrameCounter
{
private:
	LARGE_INTEGER m_nPerfFreqency;
	LARGE_INTEGER m_nPerfLatest;

	__int64 m_nFrames = 0;
	__int64 m_nFramesTime = 0;
	__int64 m_nFramesPerSecond = 0;

	std::forward_list<fps_notification_t> m_FpsSubscribers;

public:
	CFrameCounter()
	{
		::QueryPerformanceFrequency(&m_nPerfFreqency);
		::QueryPerformanceCounter(&m_nPerfLatest);
	}

	size_t Tick()
	{
		LARGE_INTEGER nPerfCurrent;
		::QueryPerformanceCounter(&nPerfCurrent);

		__int64 diff = nPerfCurrent.QuadPart - m_nPerfLatest.QuadPart;
		m_nPerfLatest.QuadPart = nPerfCurrent.QuadPart;
		
		m_nFrames += 1;
		m_nFramesTime += diff;

		if (m_nFramesTime > m_nPerfFreqency.QuadPart)
		{
			m_nFramesPerSecond = m_nFrames;
			m_nFrames = 0;
			m_nFramesTime -= m_nPerfFreqency.QuadPart;

			for (auto func : m_FpsSubscribers)
			{
				func(static_cast<size_t>(m_nFramesPerSecond));
			}
		}

		// milliseconds * 10
		return static_cast<size_t>((diff * 10000) / m_nPerfFreqency.QuadPart);
	}

	size_t GetFramesPerSecond(bool bAutoTick = false)
	{
		if (bAutoTick) Tick();
		return static_cast<size_t>(m_nFramesPerSecond);
	}

	void Subscribe(fps_notification_t target)
	{
		m_FpsSubscribers.push_front(target);
	}
};
