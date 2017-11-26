#pragma once

#include <EditorFramework/Plugin.h>
#include <QAbstractItemModel>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class EZ_EDITORFRAMEWORK_DLL ezQtDocumentTreeModel : public QAbstractItemModel
{
public:
  ezQtDocumentTreeModel(const ezDocumentObjectManager* pTree, const ezRTTI* pBaseClass, const char* szChildProperty, const char* szRootProperty = "Children");
  ~ezQtDocumentTreeModel();

  const ezDocumentObjectManager* GetDocumentTree() const { return m_pDocumentTree; }

  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex &child) const override;

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual Qt::DropActions supportedDropActions() const override;

  virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
  virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
  virtual QStringList mimeTypes() const override;
  virtual QMimeData* mimeData(const QModelIndexList& indexes) const override;

  virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  QModelIndex ComputeModelIndex(const ezDocumentObject* pObject) const;

  void SetAllowDragDrop(bool bAllow);

protected:
  virtual void TreeEventHandler(const ezDocumentObjectStructureEvent& e);

private:
  QModelIndex ComputeParent(const ezDocumentObject* pObject) const;
  ezInt32 ComputeIndex(const ezDocumentObject* pObject) const;

  void TreePropertyEventHandler(const ezDocumentObjectPropertyEvent& e);

protected:
  bool m_bAllowDragDrop;
  const ezDocumentObjectManager* m_pDocumentTree;
  const ezRTTI* m_pBaseClass;
  ezString m_sChildProperty; ///< The name of the property that contains child objects for objects below root.
  ezString m_sRootProperty; ///< The name of the property on the root object that contains our objects to display.
};

