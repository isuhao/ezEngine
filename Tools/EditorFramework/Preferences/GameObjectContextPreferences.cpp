﻿#include <PCH.h>
#include <EditorFramework/Preferences/GameObjectContextPreferences.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGameObjectContextPreferencesUser, 1, ezRTTIDefaultAllocator<ezGameObjectContextPreferencesUser>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ContextDocument", m_ContextDocument),
    EZ_MEMBER_PROPERTY("ContextObject", m_ContextObject),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

ezGameObjectContextPreferencesUser::ezGameObjectContextPreferencesUser() : ezPreferences(Domain::Document, "GameObjectContext")
{
}

ezUuid ezGameObjectContextPreferencesUser::GetContextDocument() const
{
  return m_ContextDocument;
}


void ezGameObjectContextPreferencesUser::SetContextDocument(ezUuid val)
{
  m_ContextDocument = val;
  TriggerPreferencesChangedEvent();
}

ezUuid ezGameObjectContextPreferencesUser::GetContextObject() const
{
  return m_ContextObject;
}

void ezGameObjectContextPreferencesUser::SetContextObject(ezUuid val)
{
  m_ContextObject = val;
  TriggerPreferencesChangedEvent();
}
