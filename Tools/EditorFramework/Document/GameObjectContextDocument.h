#pragma once
#include <EditorFramework/Plugin.h>
#include <EditorFramework/Document/GameObjectDocument.h>

struct EZ_EDITORFRAMEWORK_DLL ezGameObjectContextEvent
{
  enum class Type
  {
    ContextAboutToBeChanged,
    ContextChanged,
  };
  Type m_Type;
};
class EZ_EDITORFRAMEWORK_DLL ezGameObjectContextDocument : public ezGameObjectDocument
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGameObjectContextDocument, ezGameObjectDocument);
public:
  ezGameObjectContextDocument(const char* szDocumentPath, ezDocumentObjectManager* pObjectManager, bool bUseEngineConnection = true, bool bUseIPCObjectMirror = true);
  ~ezGameObjectContextDocument();

  ezStatus SetContext(ezUuid documentGuid, ezUuid objectGuid);
  ezUuid GetContextDocumentGuid() const;
  ezUuid GetContextObjectGuid() const;
  const ezDocumentObject* GetContextObject();

  mutable ezEvent<const ezGameObjectContextEvent&> m_GameObjectContextEvents;

protected:
  virtual void InitializeAfterLoading() override;

private:
  void ClearContext();

private:
  ezUuid m_ContextDocument;
  ezUuid m_ContextObject;
};
