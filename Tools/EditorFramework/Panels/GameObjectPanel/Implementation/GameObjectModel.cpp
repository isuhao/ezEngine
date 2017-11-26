﻿#include <PCH.h>
#include <EditorFramework/Panels/GameObjectPanel/GameObjectModel.moc.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Core/World/GameObject.h>
#include <ToolsFoundation/Document/Document.h>
#include <EditorFramework/Document/GameObjectDocument.h>
#include <EditorFramework/Assets/AssetCurator.h>
#include <Foundation/Strings/TranslationLookup.h>

ezQtGameObjectModel::ezQtGameObjectModel(ezGameObjectDocument* pDocument, const char* szRootProperty)
  : ezQtDocumentTreeModel(pDocument->GetObjectManager(), ezGetStaticRTTI<ezGameObject>(), "Children", szRootProperty)
{
  m_pGameObjectDocument = pDocument;

  m_pGameObjectDocument->m_GameObjectMetaData.m_DataModifiedEvent.AddEventHandler(ezMakeDelegate(&ezQtGameObjectModel::GameObjectMetaDataEventHandler, this));
  m_pGameObjectDocument->m_DocumentObjectMetaData.m_DataModifiedEvent.AddEventHandler(ezMakeDelegate(&ezQtGameObjectModel::DocumentObjectMetaDataEventHandler, this));
}

ezQtGameObjectModel::~ezQtGameObjectModel()
{
  m_pGameObjectDocument->m_GameObjectMetaData.m_DataModifiedEvent.RemoveEventHandler(ezMakeDelegate(&ezQtGameObjectModel::GameObjectMetaDataEventHandler, this));
  m_pGameObjectDocument->m_DocumentObjectMetaData.m_DataModifiedEvent.RemoveEventHandler(ezMakeDelegate(&ezQtGameObjectModel::DocumentObjectMetaDataEventHandler, this));
}

QVariant ezQtGameObjectModel::data(const QModelIndex &index, int role) const
{
  const ezDocumentObject* pObject = (const ezDocumentObject*)index.internalPointer();

  switch (role)
  {
  case Qt::DisplayRole:
    {
      ezStringBuilder sName;
      ezUuid prefabGuid;
      QIcon icon;

      m_pGameObjectDocument->QueryCachedNodeName(pObject, sName, &prefabGuid, &icon);

      const QString sQtName = QString::fromUtf8(sName.GetData());

      if (prefabGuid.IsValid())
        return QStringLiteral("[") + sQtName + QStringLiteral("]");

      return sQtName;
    }
    break;

  case Qt::DecorationRole:
    {
      auto pMeta = m_pGameObjectDocument->m_GameObjectMetaData.BeginReadMetaData(pObject->GetGuid());
      QIcon icon = pMeta->m_Icon;
      m_pGameObjectDocument->m_GameObjectMetaData.EndReadMetaData();

      return icon;
    }
    break;

  case Qt::EditRole:
    {
      ezStringBuilder sName = pObject->GetTypeAccessor().GetValue("Name").ConvertTo<ezString>();

      if (sName.IsEmpty())
      {
        auto pMeta = m_pGameObjectDocument->m_GameObjectMetaData.BeginReadMetaData(pObject->GetGuid());
        sName = pMeta->m_CachedNodeName;
        m_pGameObjectDocument->m_GameObjectMetaData.EndReadMetaData();
      }

      return QString::fromUtf8(sName.GetData());
    }
    break;

  case Qt::ToolTipRole:
    {
      auto pMeta = m_pGameObjectDocument->m_DocumentObjectMetaData.BeginReadMetaData(pObject->GetGuid());
      const ezUuid prefab = pMeta->m_CreateFromPrefab;
      m_pGameObjectDocument->m_DocumentObjectMetaData.EndReadMetaData();

      if (prefab.IsValid())
      {
        auto pInfo = ezAssetCurator::GetSingleton()->GetSubAsset(prefab);

        if (pInfo)
          return QString::fromUtf8(pInfo->m_pAssetInfo->m_sDataDirRelativePath);

        return QStringLiteral("Prefab asset could not be found");
      }

    }
    break;

  case Qt::FontRole:
    {
      auto pMeta = m_pGameObjectDocument->m_DocumentObjectMetaData.BeginReadMetaData(pObject->GetGuid());
      const bool bHidden = pMeta->m_bHidden;
      m_pGameObjectDocument->m_DocumentObjectMetaData.EndReadMetaData();

      const bool bHasName = !pObject->GetTypeAccessor().GetValue("Name").ConvertTo<ezString>().IsEmpty();

      if (bHidden || bHasName)
      {
        QFont font;

        if (bHidden)
          font.setStrikeOut(true);
        if (bHasName)
          font.setBold(true);

        return font;
      }
    }
    break;

  case Qt::ForegroundRole:
    {
      ezStringBuilder sName = pObject->GetTypeAccessor().GetValue("Name").ConvertTo<ezString>();

      auto pMeta = m_pGameObjectDocument->m_DocumentObjectMetaData.BeginReadMetaData(pObject->GetGuid());
      const bool bPrefab = pMeta->m_CreateFromPrefab.IsValid();
      m_pGameObjectDocument->m_DocumentObjectMetaData.EndReadMetaData();

      if (bPrefab)
      {
        return QColor(0, 128, 196);
      }

      if (sName.IsEmpty())
      {
        // uses an auto generated name
        return QColor(128, 128, 128);
      }

    }
    break;
  }

  return ezQtDocumentTreeModel::data(index, role);
}

bool ezQtGameObjectModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role == Qt::EditRole)
  {
    const ezDocumentObject* pObject = (const ezDocumentObject*)index.internalPointer();

    auto pMetaWrite = m_pGameObjectDocument->m_GameObjectMetaData.BeginModifyMetaData(pObject->GetGuid());

    ezStringBuilder sNewValue = value.toString().toUtf8().data();

    const ezStringBuilder sOldValue = pMetaWrite->m_CachedNodeName;

    //pMetaWrite->m_CachedNodeName.Clear();
    m_pGameObjectDocument->m_GameObjectMetaData.EndModifyMetaData(0); // no need to broadcast this change

    if (sOldValue == sNewValue && !sOldValue.IsEmpty())
      return false;

    sNewValue.Trim("[]{}() \t\r"); // forbid these

    return ezQtDocumentTreeModel::setData(index, QString::fromUtf8(sNewValue.GetData()), role);
  }

  return false;
}


void ezQtGameObjectModel::TreeEventHandler(const ezDocumentObjectStructureEvent& e)
{
  ezQtDocumentTreeModel::TreeEventHandler(e);
}

void ezQtGameObjectModel::DocumentObjectMetaDataEventHandler(const ezObjectMetaData<ezUuid, ezDocumentObjectMetaData>::EventData& e)
{
  if ((e.m_uiModifiedFlags & (ezDocumentObjectMetaData::HiddenFlag | ezDocumentObjectMetaData::PrefabFlag)) == 0)
    return;

  auto pObject = m_pGameObjectDocument->GetObjectManager()->GetObject(e.m_ObjectKey);

  if (pObject == nullptr)
  {
    // The object was destroyed due to a clear of the redo queue, i.e. it is not contained in the scene anymore.
    // So we of course won't find it in the model and thus can skip it.
    return;
  }

  // ignore all components etc.
  if (!pObject->GetTypeAccessor().GetType()->IsDerivedFrom<ezGameObject>())
    return;

  auto index = ComputeModelIndex(pObject);

  QVector<int> v;
  v.push_back(Qt::FontRole);
  dataChanged(index, index, v);
}

void ezQtGameObjectModel::GameObjectMetaDataEventHandler(const ezObjectMetaData<ezUuid, ezGameObjectMetaData>::EventData& e)
{
  if (e.m_uiModifiedFlags == 0)
    return;

    auto pObject = m_pGameObjectDocument->GetObjectManager()->GetObject(e.m_ObjectKey);

  if (pObject == nullptr)
  {
    // The object was destroyed due to a clear of the redo queue, i.e. it is not contained in the scene anymore.
    // So we of course won't find it in the model and thus can skip it.
    return;
  }

  // ignore all components etc.
  if (!pObject->GetTypeAccessor().GetType()->IsDerivedFrom<ezGameObject>())
    return;

  auto index = ComputeModelIndex(pObject);

  QVector<int> v;
  v.push_back(Qt::FontRole);
  dataChanged(index, index, v);
}



