#pragma once

#include <mmsystem.h>

#define WMU_BENCHMARK (WM_USER+10)
#define BENCHMARK_FPS 0

namespace Util
{
	class CHighResolutionTimer
	{
	private:
		bool          m_bUsePerformanceCounter;
		LARGE_INTEGER m_nPerformanceFrequency;
		LARGE_INTEGER m_nLastPerformance;
		float         m_fFramesPerSec;
		unsigned int  m_nFrames;
		unsigned int  m_nFramesTime;
		HWND          m_hTarget;

	public:
		CHighResolutionTimer() : m_fFramesPerSec(0.0f), m_nFrames(0), m_nFramesTime(0), m_hTarget(NULL)
		{
			if (QueryPerformanceFrequency(&m_nPerformanceFrequency))
			{
				LOGMSG_INFO(Logging::CLogMessage(_T("PerformanceFreqency is %d.\n"), m_nPerformanceFrequency.LowPart));
				m_bUsePerformanceCounter = true;
				QueryPerformanceCounter(&m_nLastPerformance);
			}
			else 
			{
				LOGMSG_INFO(_T("PerfomanceCounter not available!\n"));
				m_bUsePerformanceCounter   = false;
				m_nLastPerformance.LowPart = timeGetTime();
			}
		}

		~CHighResolutionTimer()
		{
		}

		int Tick()
		{
			LARGE_INTEGER current;
			int diff;

			if (m_bUsePerformanceCounter)
			{
				QueryPerformanceCounter(&current);
				diff = ((current.LowPart - m_nLastPerformance.LowPart) * 1000) / m_nPerformanceFrequency.LowPart;
			}
			else 
			{
				current.LowPart = timeGetTime();
				diff = current.LowPart - m_nLastPerformance.LowPart;
			}

			m_nLastPerformance.LowPart  = current.LowPart;

			m_nFrames ++;
			m_nFramesTime += diff;

			if (m_nFramesTime >= 1000) // update each sec
			{
				m_fFramesPerSec = (float)m_nFrames;
				m_nFramesTime  -= 1000;
				m_nFrames       = 0;
				if (m_hTarget)
					::PostMessage(m_hTarget, WMU_BENCHMARK, BENCHMARK_FPS, (LPARAM)(int)m_fFramesPerSec);
			}

			return diff;
		}

		float GetFramesPerSec(bool bAutoTick = false)
		{
			if (bAutoTick) Tick();
			return m_fFramesPerSec;
		}

		int GetCurrentFrameCount() const { return m_nFrames; }
		int GetCurrentFramesTime() const { return m_nFramesTime; }
		void SetBenchmarkTarget(HWND hWnd) { m_hTarget = hWnd; }
	};

} // of namespace
