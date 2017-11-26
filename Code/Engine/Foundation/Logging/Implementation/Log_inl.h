#pragma once

inline ezLoggingEventData::ezLoggingEventData()
{
  m_EventType = ezLogMsgType::None;
  m_uiIndentation = 0;
  m_szText = "";
  m_szTag = "";
}

#if EZ_DISABLED(EZ_COMPILE_FOR_DEVELOPMENT)

inline void ezLog::Dev(ezLogInterface* pInterface, const ezFormatString& string)
{
}

#endif

#if EZ_DISABLED(EZ_COMPILE_FOR_DEBUG)

inline void ezLog::Debug(ezLogInterface* pInterface, const ezFormatString& string)
{
}

#endif

