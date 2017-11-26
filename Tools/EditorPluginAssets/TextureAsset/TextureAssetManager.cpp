﻿#include <PCH.h>
#include <EditorPluginAssets/TextureAsset/TextureAssetManager.h>
#include <EditorPluginAssets/TextureAsset/TextureAsset.h>
#include <EditorPluginAssets/TextureAsset/TextureAssetWindow.moc.h>
#include "ToolsFoundation/Assets/AssetFileExtensionWhitelist.h"

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTextureAssetDocumentManager, 1, ezRTTIDefaultAllocator<ezTextureAssetDocumentManager>);
EZ_END_DYNAMIC_REFLECTED_TYPE

ezTextureAssetDocumentManager::ezTextureAssetDocumentManager()
{
  ezDocumentManager::s_Events.AddEventHandler(ezMakeDelegate(&ezTextureAssetDocumentManager::OnDocumentManagerEvent, this));

  // additional whitelist for non-asset files where an asset may be selected
  ezAssetFileExtensionWhitelist::AddAssetFileExtension("Texture 2D", "dds");
  ezAssetFileExtensionWhitelist::AddAssetFileExtension("Texture 2D", "color");

  // texture asset source files
  ezAssetFileExtensionWhitelist::AddAssetFileExtension("Image2D", "dds");
  ezAssetFileExtensionWhitelist::AddAssetFileExtension("Image2D", "tga");

  m_AssetDesc.m_bCanCreate = true;
  m_AssetDesc.m_sDocumentTypeName = "Texture Asset";
  m_AssetDesc.m_sFileExtension = "ezTextureAsset";
  m_AssetDesc.m_sIcon = ":/AssetIcons/Texture_2D.png";
  m_AssetDesc.m_pDocumentType = ezGetStaticRTTI<ezTextureAssetDocument>();
  m_AssetDesc.m_pManager = this;
}

ezTextureAssetDocumentManager::~ezTextureAssetDocumentManager()
{
  ezDocumentManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezTextureAssetDocumentManager::OnDocumentManagerEvent, this));
}


ezBitflags<ezAssetDocumentFlags> ezTextureAssetDocumentManager::GetAssetDocumentTypeFlags(const ezDocumentTypeDescriptor* pDescriptor) const
{
  return ezAssetDocumentFlags::AutoThumbnailOnTransform;
}

void ezTextureAssetDocumentManager::OnDocumentManagerEvent(const ezDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
  case ezDocumentManager::Event::Type::DocumentWindowRequested:
    {
      if (e.m_pDocument->GetDynamicRTTI() == ezGetStaticRTTI<ezTextureAssetDocument>())
      {
        ezQtTextureAssetDocumentWindow* pDocWnd = new ezQtTextureAssetDocumentWindow(static_cast<ezTextureAssetDocument*>(e.m_pDocument));
      }
    }
    break;
  }
}

ezStatus ezTextureAssetDocumentManager::InternalCreateDocument(const char* szDocumentTypeName, const char* szPath, ezDocument*& out_pDocument)
{
  out_pDocument = new ezTextureAssetDocument(szPath);

  return ezStatus(EZ_SUCCESS);
}

void ezTextureAssetDocumentManager::InternalGetSupportedDocumentTypes(ezDynamicArray<const ezDocumentTypeDescriptor*>& inout_DocumentTypes) const
{
  inout_DocumentTypes.PushBack(&m_AssetDesc);
}



