#pragma once

#include <GuiFoundation/Basics.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>

class QHBoxLayout;
class QPushButton;
class QMenu;

class EZ_GUIFOUNDATION_DLL ezQtAddSubElementButton : public ezQtPropertyWidget
{
  Q_OBJECT

public:
  ezQtAddSubElementButton();

protected:
  virtual void DoPrepareToDie() override {}

private slots:
  void onMenuAboutToShow();
  void on_Button_clicked();
  void OnMenuAction();

private:
  virtual void OnInit() override;
  void OnAction(const ezRTTI* pRtti);

  QMenu* CreateCategoryMenu(const char* szCategory, ezMap<ezString, QMenu*>& existingMenus);

  QHBoxLayout* m_pLayout;
  QPushButton* m_pButton;

  ezSet<const ezRTTI*> m_SupportedTypes;

  bool m_bNoMoreElementsAllowed = false;
  QMenu* m_pMenu;
  ezUInt32 m_uiMaxElements = 0; // 0 means unlimited
  bool m_bPreventDuplicates = false;
  const ezConstrainPointerAttribute* m_pConstraint = nullptr;
};
