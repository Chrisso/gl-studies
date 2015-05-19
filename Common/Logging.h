#ifndef __LOGGING_H__
#define __LOGGING_H__

#pragma once

#include <atlsimpcoll.h>

namespace Logging
{
	enum LOG_LEVEL
	{
		LOG_LEVEL_NONE = 0,
		LOG_LEVEL_DEBUG = 1,
		LOG_LEVEL_INFO = 2,
		LOG_LEVEL_WARN = 3,
		LOG_LEVEL_ERROR = 4,
		LOG_LEVEL_FATAL = 5
	};

	class CLogMessage
	{
	public:
		CString m_szMessage;

	public:
		CLogMessage(LPCTSTR szMsg) : m_szMessage(szMsg) { }
		CLogMessage(const CLogMessage& msg) : m_szMessage(msg.m_szMessage) { }
		CLogMessage(LPCTSTR szFormat, ...) 
		{
			va_list args;
			va_start(args, szFormat);
			m_szMessage.FormatV(szFormat, args);
			va_end(args);
		}

		operator LPCTSTR() const { return (LPCTSTR)m_szMessage; }
		CLogMessage& operator=(const CLogMessage& msg) { m_szMessage = msg.m_szMessage; return *this; }
		bool operator==(const CLogMessage& msg) const { return m_szMessage.Compare(msg.m_szMessage) == 0; }
	};

	class ILogTarget
	{
	public:
		virtual ~ILogTarget() { }
		virtual bool IsEnabled(LOG_LEVEL lvl) = 0;
		virtual void Append(LPCTSTR szMsg) = 0;
	};

	class CLogTargetBase : public ILogTarget
	{
	protected:
		LOG_LEVEL m_nLevel;

	public:
		CLogTargetBase(LOG_LEVEL lvl) : m_nLevel(lvl) { }
		virtual ~CLogTargetBase() {}
		virtual bool IsEnabled(LOG_LEVEL lvl) { return m_nLevel <= lvl; }
	};

	class CLogTargetDebugger : public CLogTargetBase
	{
	public:
		CLogTargetDebugger(LOG_LEVEL lvl) : CLogTargetBase(lvl) { }
		virtual ~CLogTargetDebugger() { }
		virtual void Append(LPCTSTR szMsg) { ::OutputDebugString(szMsg); }
	};

	class CLogTargetMessageBox : public CLogTargetBase
	{
	public:
		CLogTargetMessageBox(LOG_LEVEL lvl) : CLogTargetBase(lvl) { }
		virtual ~CLogTargetMessageBox() { }
		virtual void Append(LPCTSTR szMsg) { ::AtlMessageBox(NULL, szMsg); }
	};

	class CLogger
	{
	private:
		DWORD m_dwLogStart;
		CSimpleArray <ILogTarget*> m_pTargets;

	public:
		CLogger();
		~CLogger();
		
		void AddTarget(ILogTarget* pTarget);
		void Log(LOG_LEVEL lvl, LPCTSTR szMsg, LPCTSTR szFile, LPCTSTR szFunction, int nLine);
	};

	class CLoggerFactory
	{
	private:
		static CLogger m_SingletonInstance;	// TODO: write a real singleton/ factory pattern implementation

	public:
		static CLogger* getDefaultInstance() { return &m_SingletonInstance; }
	};
}

#define __LOGMSG(lvl, msg, file, func, line) Logging::CLoggerFactory::getDefaultInstance()->Log(lvl, msg, file, func, line)

#ifdef UNICODE
	#define LOGMSG(lvl, msg)  __LOGMSG(lvl, msg, __FILEW__, __FUNCTIONW__, __LINE__)
	#define LOGMSG_DEBUG(msg) __LOGMSG(Logging::LOG_LEVEL_DEBUG, msg, __FILEW__, __FUNCTIONW__, __LINE__)
	#define LOGMSG_INFO(msg)  __LOGMSG(Logging::LOG_LEVEL_INFO, msg, __FILEW__, __FUNCTIONW__, __LINE__)
	#define LOGMSG_WARN(msg)  __LOGMSG(Logging::LOG_LEVEL_WARN, msg, __FILEW__, __FUNCTIONW__, __LINE__)
	#define LOGMSG_ERROR(msg) __LOGMSG(Logging::LOG_LEVEL_ERROR, msg, __FILEW__, __FUNCTIONW__, __LINE__)
	#define LOGMSG_FATAL(msg) __LOGMSG(Logging::LOG_LEVEL_FATAL, msg, __FILEW__, __FUNCTIONW__, __LINE__)
#else
	#define LOGMSG(lvl, msg)  __LOGMSG(lvl, msg, __FILE__, __FUNCTION__, __LINE__)
	#define LOGMSG_DEBUG(msg) __LOGMSG(Logging::LOG_LEVEL_DEBUG, msg, __FILE__, __FUNCTION__, __LINE__)
	#define LOGMSG_INFO(msg)  __LOGMSG(Logging::LOG_LEVEL_INFO, msg, __FILE__, __FUNCTION__, __LINE__)
	#define LOGMSG_WARN(msg)  __LOGMSG(Logging::LOG_LEVEL_WARN, msg, __FILE__, __FUNCTION__, __LINE__)
	#define LOGMSG_ERROR(msg) __LOGMSG(Logging::LOG_LEVEL_ERROR, msg, __FILE__, __FUNCTION__, __LINE__)
	#define LOGMSG_FATAL(msg) __LOGMSG(Logging::LOG_LEVEL_FATAL, msg, __FILE__, __FUNCTION__, __LINE__)
#endif // UNICODE

#endif // __LOGGING_H__
