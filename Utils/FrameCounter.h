#pragma once

#define WM_APP_BENCHMARK (WM_APP+10)

class CFrameCounter
{
private:
	LARGE_INTEGER m_nPerfFreqency;
	LARGE_INTEGER m_nPerfLatest;

	__int64 m_nFrames = 0;
	__int64 m_nFramesTime = 0;
	__int64 m_nFramesPerSecond = 0;

	HWND m_hTarget;

public:
	CFrameCounter() : m_hTarget(NULL)
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

			if (m_hTarget)
				::PostMessage(m_hTarget, WM_APP_BENCHMARK, 0, (LPARAM)m_nFramesPerSecond);
		}

		// milliseconds * 10
		return static_cast<size_t>((diff * 10000) / m_nPerfFreqency.QuadPart);
	}

	size_t GetFramesPerSecond(bool bAutoTick = false)
	{
		if (bAutoTick) Tick();
		return static_cast<size_t>(m_nFramesPerSecond);
	}

	void SetTargetWindow(HWND hWnd)
	{
		m_hTarget = hWnd;
	}
};
