#pragma once

#include <GuiFoundation/Basics.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <QWidget>

class QGridLayout;
class ezDocument;
class ezQtManipulatorLabel;
struct ezManipulatorManagerEvent;

class EZ_GUIFOUNDATION_DLL ezQtTypeWidget : public QWidget
{
  Q_OBJECT
public:
  ezQtTypeWidget(QWidget* pParent, ezQtPropertyGridWidget* pGrid, const ezRTTI* pType);
  ~ezQtTypeWidget();
  void SetSelection(const ezHybridArray<ezPropertySelection, 8>& items);
  const ezHybridArray<ezPropertySelection, 8>& GetSelection() const { return m_Items; }
  const ezRTTI* GetType() const { return m_pType; }
  void PrepareToDie();

private:
  void BuildUI(const ezRTTI* pType);
  void BuildUI(const ezRTTI* pType, const ezMap<ezString, const ezManipulatorAttribute*>& manipulatorMap);

  void PropertyEventHandler(const ezDocumentObjectPropertyEvent& e);
  void CommandHistoryEventHandler(const ezCommandHistoryEvent& e);
  void ManipulatorManagerEventHandler(const ezManipulatorManagerEvent& e);

  void UpdateProperty(const ezDocumentObject* pObject, const ezString& sProperty);
  void FlushQueuedChanges();
  void UpdatePropertyMetaState();

private:
  ezPropertyEventHandler m_EventHandler;
  bool m_bUndead;
  ezQtPropertyGridWidget* m_pGrid;
  const ezRTTI* m_pType;
  ezHybridArray<ezPropertySelection, 8> m_Items;

  struct PropertyWidgetData
  {
    ezQtPropertyWidget* m_pWidget;
    ezQtManipulatorLabel* m_pLabel;
    ezString m_sOriginalLabelText;
  };

  QGridLayout* m_pLayout;
  ezMap<ezString, PropertyWidgetData> m_PropertyWidgets;
  ezHybridArray<ezString, 1> m_QueuedChanges;

};


