#include <PCH.h>
#include <EditorPluginAssets/TextureAsset/TextureAsset.h>
#include <EditorPluginAssets/TextureAsset/TextureAssetObjects.h>
#include <EditorPluginAssets/TextureAsset/TextureAssetManager.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <EditorFramework/Assets/AssetCurator.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Image/Formats/DdsFileFormat.h>
#include <Foundation/Image/ImageConversion.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Core/Assets/AssetFileHeader.h>
#include <QStringList>
#include <QTextStream>
#include <Foundation/IO/OSFile.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTextureAssetDocument, 4, ezRTTINoAllocator);
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezTextureChannelMode, 1)
EZ_ENUM_CONSTANTS(ezTextureChannelMode::RGB, ezTextureChannelMode::Red, ezTextureChannelMode::Green, ezTextureChannelMode::Blue, ezTextureChannelMode::Alpha)
EZ_END_STATIC_REFLECTED_ENUM()

ezTextureAssetDocument::ezTextureAssetDocument(const char* szDocumentPath)
  : ezSimpleAssetDocument<ezTextureAssetProperties>(szDocumentPath, true)
{
  m_iTextureLod = -1;
}

ezStatus ezTextureAssetDocument::RunTexConv(const char* szTargetFile, const ezAssetFileHeader& AssetHeader, bool bUpdateThumbnail)
{
  const ezTextureAssetProperties* pProp = GetProperties();

  QStringList arguments;
  ezStringBuilder temp;

  // Asset Version
  {
    arguments << "-assetVersion";
    arguments << ezConversionUtils::ToString(AssetHeader.GetFileVersion(), temp).GetData();
  }

  // Asset Hash
  {
    const ezUInt64 uiHash64 = AssetHeader.GetFileHash();
    const ezUInt32 uiHashLow32 = uiHash64 & 0xFFFFFFFF;
    const ezUInt32 uiHashHigh32 = (uiHash64 >> 32) & 0xFFFFFFFF;

    temp.Format("{0}", ezArgU(uiHashLow32, 8, true, 16, true));
    arguments << "-assetHashLow";
    arguments << temp.GetData();

    temp.Format("{0}", ezArgU(uiHashHigh32, 8, true, 16, true));
    arguments << "-assetHashHigh";
    arguments << temp.GetData();
  }


  arguments << "-out";
  arguments << szTargetFile;

  const ezStringBuilder sThumbnail = GetThumbnailFilePath();
  if (bUpdateThumbnail)
  {
    // Thumbnail
    const ezStringBuilder sDir = sThumbnail.GetFileDirectory();
    ezOSFile::CreateDirectoryStructure(sDir);

    arguments << "-thumbnail";
    arguments << QString::fromUtf8(sThumbnail.GetData());
  }

  arguments << "-channels";
  arguments << ezConversionUtils::ToString(pProp->GetNumChannels(), temp).GetData();

  if (pProp->m_bMipmaps)
    arguments << "-mipmaps";

  if (pProp->m_bCompression)
    arguments << "-compress";

  if (pProp->IsSRGB())
    arguments << "-srgb";

  if (pProp->IsHDR())
    arguments << "-hdr";

  if (pProp->m_bPremultipliedAlpha)
    arguments << "-premulalpha";

  //if (pProp->IsTextureCube())
  //  arguments << "-cubemap";

  arguments << "-addressU" << QString::number(pProp->m_AddressModeU.GetValue());
  arguments << "-addressV" << QString::number(pProp->m_AddressModeV.GetValue());
  arguments << "-addressW" << QString::number(pProp->m_AddressModeW.GetValue());
  arguments << "-filter" << QString::number(pProp->m_TextureFilter.GetValue());

  const ezInt32 iNumInputFiles = pProp->GetNumInputFiles();
  for (ezInt32 i = 0; i < iNumInputFiles; ++i)
  {
    temp.Format("-in{0}", i);

    if (ezStringUtils::IsNullOrEmpty(pProp->GetInputFile(i)))
      break;

    arguments << temp.GetData();
    arguments << QString(pProp->GetAbsoluteInputFilePath(i).GetData());
  }

  switch (pProp->GetChannelMapping())
  {
  case ezTexture2DChannelMappingEnum::R1:
    {
      arguments << "-r";
      arguments << "in0.r"; // always linear
    }
    break;
  case ezTexture2DChannelMappingEnum::RG1:
    {
      arguments << "-rg";
      arguments << "in0.rg"; // always linear
    }
    break;
  case ezTexture2DChannelMappingEnum::R1_G2:
    {
      arguments << "-r";
      arguments << "in0.r";
      arguments << "-g";
      arguments << "in1.y"; // always linear
    }
    break;
  case ezTexture2DChannelMappingEnum::RGB1:
    {
      arguments << "-rgb";
      arguments << "in0.rgb";
    }
    break;
  case ezTexture2DChannelMappingEnum::RGB1_ABLACK:
    {
      arguments << "-rgb";
      arguments << "in0.rgb";
      arguments << "-a";
      arguments << "black";
    }
    break;
  case ezTexture2DChannelMappingEnum::R1_G2_B3:
    {
      arguments << "-r";
      arguments << "in0.r";
      arguments << "-g";
      arguments << "in1.r";
      arguments << "-b";
      arguments << "in2.r";
    }
    break;
  case ezTexture2DChannelMappingEnum::RGBA1:
    {
      arguments << "-rgba";
      arguments << "in0.rgba";
    }
    break;
  case ezTexture2DChannelMappingEnum::RGB1_A2:
    {
      arguments << "-rgb";
      arguments << "in0.rgb";
      arguments << "-a";
      arguments << "in1.r";
    }
    break;
  case ezTexture2DChannelMappingEnum::R1_G2_B3_A4:
    {
      arguments << "-r";
      arguments << "in0.r";
      arguments << "-g";
      arguments << "in1.r";
      arguments << "-b";
      arguments << "in2.r";
      arguments << "-a";
      arguments << "in3.r";
    }
    break;
  }

  ezStringBuilder cmd;
  for (ezInt32 i = 0; i < arguments.size(); ++i)
    cmd.Append(" ", arguments[i].toUtf8().data());

  ezLog::Debug("TexConv.exe{0}", cmd);

  EZ_SUCCEED_OR_RETURN(ezQtEditorApp::GetSingleton()->ExecuteTool("TexConv.exe", arguments, 60, ezLog::GetThreadLocalLogSystem()));

  if (bUpdateThumbnail)
  {
    ezUInt64 uiThumbnailHash = ezAssetCurator::GetSingleton()->GetAssetReferenceHash(GetGuid());
    EZ_ASSERT_DEV(uiThumbnailHash != 0, "Thumbnail hash should never be zero when reaching this point!");
    ezAssetFileHeader assetThumbnailHeader;
    assetThumbnailHeader.SetFileHashAndVersion(uiThumbnailHash, GetAssetTypeVersion());
    AppendThumbnailInfo(sThumbnail, assetThumbnailHeader);
    InvalidateAssetThumbnail();
  }

  return ezStatus(EZ_SUCCESS);
}

ezStatus ezTextureAssetDocument::InternalTransformAsset(const char* szTargetFile, const char* szOutputTag, const char* szPlatform, const ezAssetFileHeader& AssetHeader, bool bTriggeredManually)
{
  EZ_ASSERT_DEV(ezStringUtils::IsEqual(szPlatform, "PC"), "Platform '{0}' is not supported", szPlatform);
  const bool bUpdateThumbnail = ezStringUtils::IsEqual(szPlatform, "PC");

  ezStatus result = RunTexConv(szTargetFile, AssetHeader, bUpdateThumbnail);

  ezFileStats stat;
  if (ezOSFile::GetFileStats(szTargetFile, stat).Succeeded() && stat.m_uiFileSize == 0)
  {
    // if the file was touched, but nothing written to it, delete the file
    // might happen if TexConv crashed or had an error
    ezOSFile::DeleteFile(szTargetFile);
    result.m_Result = EZ_FAILURE;
  }

  return result;
}

const char* ezTextureAssetDocument::QueryAssetType() const
{
  return "Texture 2D";
}

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTextureAssetDocumentGenerator, 1, ezRTTIDefaultAllocator<ezTextureAssetDocumentGenerator>)
EZ_END_DYNAMIC_REFLECTED_TYPE

ezTextureAssetDocumentGenerator::ezTextureAssetDocumentGenerator()
{
  AddSupportedFileType("tga");
  AddSupportedFileType("dds");
  AddSupportedFileType("jpg");
  AddSupportedFileType("jpeg");
  AddSupportedFileType("hdr");
  AddSupportedFileType("png");
}

ezTextureAssetDocumentGenerator::~ezTextureAssetDocumentGenerator()
{

}

void ezTextureAssetDocumentGenerator::GetImportModes(const char* szParentDirRelativePath, ezHybridArray<ezAssetDocumentGenerator::Info, 4>& out_Modes) const
{
  ezStringBuilder baseOutputFile = szParentDirRelativePath;

  const ezStringBuilder baseFilename = baseOutputFile.GetFileName();
  const bool isHDR = ezPathUtils::HasExtension(szParentDirRelativePath, "hdr");

  baseOutputFile.ChangeFileExtension(GetDocumentExtension());

  /// \todo make this configurable
  const bool isNormalMap = !isHDR && (baseFilename.EndsWith_NoCase("_n") ||
    baseFilename.EndsWith_NoCase("normal") || baseFilename.EndsWith_NoCase("normals") || baseFilename.EndsWith_NoCase("norm"));

  if (isHDR)
  {
    {
      ezAssetDocumentGenerator::Info& info = out_Modes.ExpandAndGetRef();
      info.m_Priority = ezAssetDocGeneratorPriority::DefaultPriority;
      info.m_sName = "TextureImport.HDR";
      info.m_sOutputFileParentRelative = baseOutputFile;
      info.m_sIcon = ":/AssetIcons/Texture_2D.png";
    }

  }
  else
  {
    {
      ezAssetDocumentGenerator::Info& info = out_Modes.ExpandAndGetRef();
      info.m_Priority = ezAssetDocGeneratorPriority::DefaultPriority;
      info.m_sName = "TextureImport.Diffuse";
      info.m_sOutputFileParentRelative = baseOutputFile;
      info.m_sIcon = ":/AssetIcons/Texture_2D.png";
    }

    {
      ezAssetDocumentGenerator::Info& info = out_Modes.ExpandAndGetRef();
      info.m_Priority = isNormalMap ? ezAssetDocGeneratorPriority::HighPriority : ezAssetDocGeneratorPriority::LowPriority;
      info.m_sName = "TextureImport.Normal";
      info.m_sOutputFileParentRelative = baseOutputFile;
      info.m_sIcon = ":/AssetIcons/Texture_Normals.png";
    }

    {
      ezAssetDocumentGenerator::Info& info = out_Modes.ExpandAndGetRef();
      info.m_Priority = ezAssetDocGeneratorPriority::LowPriority;
      info.m_sName = "TextureImport.Linear";
      info.m_sOutputFileParentRelative = baseOutputFile;
      info.m_sIcon = ":/AssetIcons/Texture_Linear.png";
    }
  }
}

ezStatus ezTextureAssetDocumentGenerator::Generate(const char* szDataDirRelativePath, const ezAssetDocumentGenerator::Info& info, ezDocument*& out_pGeneratedDocument)
{
  auto pApp = ezQtEditorApp::GetSingleton();

  out_pGeneratedDocument = pApp->CreateOrOpenDocument(true, info.m_sOutputFileAbsolute, false, false);
  if (out_pGeneratedDocument == nullptr)
    return ezStatus("Could not create target document");

  ezTextureAssetDocument* pAssetDoc = ezDynamicCast<ezTextureAssetDocument*>(out_pGeneratedDocument);
  if (pAssetDoc == nullptr)
    return ezStatus("Target document is not a valid ezTextureAssetDocument");

  auto& accessor = pAssetDoc->GetPropertyObject()->GetTypeAccessor();
  accessor.SetValue("Input1", szDataDirRelativePath);
  accessor.SetValue("ChannelMapping", (int)ezTexture2DChannelMappingEnum::RGB1);

  if (info.m_sName == "TextureImport.Diffuse")
  {
    accessor.SetValue("Usage", (int)ezTexture2DUsageEnum::Diffuse);
  }
  else if (info.m_sName == "TextureImport.Normal")
  {
    accessor.SetValue("Usage", (int)ezTexture2DUsageEnum::NormalMap);
  }
  else if (info.m_sName == "TextureImport.HDR")
  {
    accessor.SetValue("Usage", (int)ezTexture2DUsageEnum::HDR);
  }
  else if (info.m_sName == "TextureImport.Linear")
  {
    accessor.SetValue("Usage", (int)ezTexture2DUsageEnum::Other_Linear);
  }

  return ezStatus(EZ_SUCCESS);
}




