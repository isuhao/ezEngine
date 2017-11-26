#include <PCH.h>
#include <Foundation/Logging/LogEntry.h>

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezLogMsgType, 1)
  EZ_BITFLAGS_CONSTANTS(ezLogMsgType::BeginGroup, ezLogMsgType::EndGroup, ezLogMsgType::None)
  EZ_BITFLAGS_CONSTANTS(ezLogMsgType::ErrorMsg, ezLogMsgType::SeriousWarningMsg, ezLogMsgType::WarningMsg, ezLogMsgType::SuccessMsg, ezLogMsgType::InfoMsg, ezLogMsgType::DevMsg, ezLogMsgType::DebugMsg, ezLogMsgType::All)
EZ_END_STATIC_REFLECTED_ENUM();

EZ_BEGIN_STATIC_REFLECTED_TYPE(ezLogEntry, ezNoBase, 1, ezRTTIDefaultAllocator<ezLogEntry>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Msg", m_sMsg),
    EZ_MEMBER_PROPERTY("Tag", m_sTag),
    EZ_ENUM_MEMBER_PROPERTY("Type", ezLogMsgType, m_Type),
    EZ_MEMBER_PROPERTY("Indentation", m_uiIndentation),
  }
  EZ_END_PROPERTIES
}
EZ_END_STATIC_REFLECTED_TYPE

ezLogEntry::ezLogEntry()
{
}

ezLogEntry::ezLogEntry(const ezLoggingEventData& le)
{
  m_sMsg = le.m_szText;
  m_sTag = le.m_szTag;
  m_Type = le.m_EventType;
  m_uiIndentation = le.m_uiIndentation;
}

ezLogEntryDelegate::ezLogEntryDelegate(Callback callback, ezLogMsgType::Enum LogLevel) : m_Callback(callback)
{
  SetLogLevel(LogLevel);
}

void ezLogEntryDelegate::HandleLogMessage(const ezLoggingEventData& le)
{
  ezLogEntry e(le);
  m_Callback(e);
}

EZ_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_LogEntry);
