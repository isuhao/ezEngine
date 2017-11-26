﻿#include <PCH.h>
#include <EditorPluginAssets/MaterialAsset/ShaderTypeRegistry.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

EZ_IMPLEMENT_SINGLETON(ezShaderTypeRegistry);

EZ_BEGIN_SUBSYSTEM_DECLARATION(EditorFramework, ShaderTypeRegistry)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "PluginAssets", "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORE_STARTUP
  {
    EZ_DEFAULT_NEW(ezShaderTypeRegistry);
  }

  ON_CORE_SHUTDOWN
  {
    ezShaderTypeRegistry* pDummy = ezShaderTypeRegistry::GetSingleton();
    EZ_DEFAULT_DELETE(pDummy);
  }

  ON_ENGINE_STARTUP
  {
  }

  ON_ENGINE_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION


namespace
{
  struct PermutationVarConfig
  {
    ezVariant m_DefaultValue;
    const ezRTTI* m_pType;
  };

  static ezHashTable<const char*, const ezRTTI*> s_NameToTypeTable;

  void InitializeTables()
  {
    if (!s_NameToTypeTable.IsEmpty())
      return;

    s_NameToTypeTable.Insert("float", ezGetStaticRTTI<float>());
    s_NameToTypeTable.Insert("float2", ezGetStaticRTTI<ezVec2>());
    s_NameToTypeTable.Insert("float3", ezGetStaticRTTI<ezVec3>());
    s_NameToTypeTable.Insert("float4", ezGetStaticRTTI<ezVec4>());
    s_NameToTypeTable.Insert("int", ezGetStaticRTTI<int>());
    s_NameToTypeTable.Insert("int2", ezGetStaticRTTI<ezVec2I32>());
    s_NameToTypeTable.Insert("int3", ezGetStaticRTTI<ezVec3I32>());
    s_NameToTypeTable.Insert("int4", ezGetStaticRTTI<ezVec4I32>());
    s_NameToTypeTable.Insert("uint", ezGetStaticRTTI<ezUInt32>());
    s_NameToTypeTable.Insert("uint2", ezGetStaticRTTI<ezVec2U32>());
    s_NameToTypeTable.Insert("uint3", ezGetStaticRTTI<ezVec3U32>());
    s_NameToTypeTable.Insert("uint4", ezGetStaticRTTI<ezVec4U32>());
    s_NameToTypeTable.Insert("bool", ezGetStaticRTTI<bool>());
    s_NameToTypeTable.Insert("Color", ezGetStaticRTTI<ezColor>());
    /// \todo Are we going to support linear UB colors ?
    s_NameToTypeTable.Insert("Texture2D", ezGetStaticRTTI<ezString>());
    s_NameToTypeTable.Insert("Texture3D", ezGetStaticRTTI<ezString>());
    s_NameToTypeTable.Insert("TextureCube", ezGetStaticRTTI<ezString>());
  }

  static ezHashTable<ezString, PermutationVarConfig> s_PermutationVarConfigs;

  const ezRTTI* GetPermutationType(ezShaderParser::ParameterDefinition& def)
  {
    EZ_ASSERT_DEV(def.m_sType.IsEqual("Permutation"), "");

    PermutationVarConfig* pConfig = nullptr;
    if (s_PermutationVarConfigs.TryGetValue(def.m_sName, pConfig))
    {
      return pConfig->m_pType;
    }

    ezStringBuilder sTemp;
    sTemp.Format("Shaders/PermutationVars/{0}.ezPermVar", def.m_sName);

    ezString sPath = sTemp;
    ezQtEditorApp::GetSingleton()->MakeDataDirectoryRelativePathAbsolute(sPath);

    ezFileReader file;
    if (file.Open(sPath).Failed())
    {
      return nullptr;
    }

    sTemp.ReadAll(file);

    ezVariant defaultValue;
    ezHybridArray<ezHashedString, 16> enumValues;

    ezShaderParser::ParsePermutationVarConfig(def.m_sName, sTemp, defaultValue, enumValues);
    if (defaultValue.IsValid())
    {
      pConfig = &(s_PermutationVarConfigs[def.m_sName]);
      pConfig->m_DefaultValue = defaultValue;

      if (defaultValue.IsA<bool>())
      {
        pConfig->m_pType = ezGetStaticRTTI<bool>();
      }
      else
      {
        ezReflectedTypeDescriptor descEnum;
        descEnum.m_sTypeName = def.m_sName;
        descEnum.m_sPluginName = "ShaderTypes";
        descEnum.m_sParentTypeName = ezGetStaticRTTI<ezEnumBase>()->GetTypeName();
        descEnum.m_Flags = ezTypeFlags::IsEnum | ezTypeFlags::Phantom;
        descEnum.m_uiTypeSize = 0;
        descEnum.m_uiTypeVersion = 1;

        ezArrayPtr<ezPropertyAttribute* const> noAttributes;

        ezStringBuilder sEnumName;
        sEnumName.Format("{0}::Default", def.m_sName);

        descEnum.m_Properties.PushBack(ezReflectedPropertyDescriptor(sEnumName, defaultValue.Get<ezUInt32>(), noAttributes));

        for (ezUInt32 i = 0; i < enumValues.GetCount(); ++i)
        {
          if (enumValues[i].IsEmpty())
            continue;

          ezStringBuilder sEnumName;
          sEnumName.Format("{0}::{1}", def.m_sName, enumValues[i]);

          descEnum.m_Properties.PushBack(ezReflectedPropertyDescriptor(sEnumName, (ezUInt32)i, noAttributes));
        }

        pConfig->m_pType = ezPhantomRttiManager::RegisterType(descEnum);
      }

      return pConfig->m_pType;
    }

    return nullptr;
  }

  const ezRTTI* GetType(ezShaderParser::ParameterDefinition& def)
  {
    InitializeTables();

    if (def.m_sType.IsEqual("Permutation"))
    {
      return GetPermutationType(def);
    }

    const ezRTTI* pType = nullptr;
    s_NameToTypeTable.TryGetValue(def.m_sType.GetData(), pType);
    return pType;
  }

  void AddAttributes(ezShaderParser::ParameterDefinition& def, const ezRTTI* pType, ezHybridArray<ezPropertyAttribute*, 2>& attributes)
  {
    if (def.m_sType.StartsWith_NoCase("texture"))
    {
    }
    else if (def.m_sType.StartsWith_NoCase("permutation"))
    {
      attributes.PushBack(EZ_DEFAULT_NEW(ezCategoryAttribute, "Permutation"));
    }
    else
    {
      attributes.PushBack(EZ_DEFAULT_NEW(ezCategoryAttribute, "Constant"));
    }

    if (def.m_sType.IsEqual("Texture2D"))
    {
      attributes.PushBack(EZ_DEFAULT_NEW(ezCategoryAttribute, "Texture 2D"));
      attributes.PushBack(EZ_DEFAULT_NEW(ezAssetBrowserAttribute, "Texture 2D"));
    }
    else if (def.m_sType.IsEqual("Texture3D"))
    {
      attributes.PushBack(EZ_DEFAULT_NEW(ezCategoryAttribute, "Texture 3D"));
      attributes.PushBack(EZ_DEFAULT_NEW(ezAssetBrowserAttribute, "Texture 3D"));
    }
    else if (def.m_sType.IsEqual("TextureCube"))
    {
      attributes.PushBack(EZ_DEFAULT_NEW(ezCategoryAttribute, "Texture Cube"));
      attributes.PushBack(EZ_DEFAULT_NEW(ezAssetBrowserAttribute, "Texture Cube"));
    }

    for (auto& attributeDef : def.m_Attributes)
    {
      float fValues[4] = {};
      ezUInt32 uiNumFloats = ezConversionUtils::ExtractFloatsFromString(attributeDef.m_sValue, 4, fValues);

      if (attributeDef.m_sName.IsEqual("Default"))
      {
        ///\todo: this needs a proper implementation for types other than float
        if (pType == ezGetStaticRTTI<float>())
        {
          if (uiNumFloats >= 1)
          {
            attributes.PushBack(EZ_DEFAULT_NEW(ezDefaultValueAttribute, fValues[0]));
          }
        }
        else if (pType == ezGetStaticRTTI<ezColor>())
        {
          if (uiNumFloats >= 3)
          {
            ezColor color(fValues[0], fValues[1], fValues[2]);
            if (uiNumFloats == 4)
            {
              color.a = fValues[3];
            }

            attributes.PushBack(EZ_DEFAULT_NEW(ezDefaultValueAttribute, color));

            // always expose the alpha channel for color properties
            attributes.PushBack(EZ_DEFAULT_NEW(ezExposeColorAlphaAttribute));
          }
        }
        else if (pType == ezGetStaticRTTI<ezString>())
        {
          attributes.PushBack(EZ_DEFAULT_NEW(ezDefaultValueAttribute, attributeDef.m_sValue));
        }
      }
      else if (attributeDef.m_sName.IsEqual("Clamp"))
      {
        if (pType == ezGetStaticRTTI<float>())
        {
          if (uiNumFloats >= 2)
          {
            attributes.PushBack(EZ_DEFAULT_NEW(ezClampValueAttribute, fValues[0], fValues[1]));
          }
        }
      }
    }
  }
}

ezShaderTypeRegistry::ezShaderTypeRegistry()
  : m_SingletonRegistrar(this)
{
  ezShaderTypeRegistry::GetSingleton();
  ezReflectedTypeDescriptor desc;
  desc.m_sTypeName = "ezShaderTypeBase";
  desc.m_sPluginName = "ShaderTypes";
  desc.m_sParentTypeName = ezGetStaticRTTI<ezReflectedClass>()->GetTypeName();
  desc.m_Flags = ezTypeFlags::Phantom | ezTypeFlags::Abstract | ezTypeFlags::Class;
  desc.m_uiTypeSize = 0;
  desc.m_uiTypeVersion = 2;

  m_pBaseType = ezPhantomRttiManager::RegisterType(desc);

  ezPhantomRttiManager::s_Events.AddEventHandler(ezMakeDelegate(&ezShaderTypeRegistry::PhantomTypeRegistryEventHandler, this));
}


ezShaderTypeRegistry::~ezShaderTypeRegistry()
{
  ezPhantomRttiManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezShaderTypeRegistry::PhantomTypeRegistryEventHandler, this));
}

const ezRTTI* ezShaderTypeRegistry::GetShaderType(const char* szShaderPath)
{
  if (ezStringUtils::IsNullOrEmpty(szShaderPath))
    return nullptr;

  ezStringBuilder sShaderPath = szShaderPath;
  sShaderPath.MakeCleanPath();

  if (sShaderPath.IsAbsolutePath())
  {
    if (!ezQtEditorApp::GetSingleton()->MakePathDataDirectoryRelative(sShaderPath))
    {
      ezLog::Error("Could not make shader path '{0}' relative!", sShaderPath);
    }
  }

  auto it = m_ShaderTypes.Find(sShaderPath);
  if (it.IsValid())
  {
    ezFileStats Stats;
    if (ezOSFile::GetFileStats(it.Value().m_sAbsShaderPath, Stats).Succeeded() && !Stats.m_LastModificationTime.Compare(it.Value().m_fileModifiedTime, ezTimestamp::CompareMode::FileTimeEqual))
    {
      UpdateShaderType(it.Value());
    }
  }
  else
  {
    ezStringBuilder sAbsPath = szShaderPath;
    {
      if (!ezQtEditorApp::GetSingleton()->MakeDataDirectoryRelativePathAbsolute(sAbsPath))
      {
        ezLog::Error("Can't make path absolute: '{0}'", szShaderPath);
        return nullptr;
      }
      sAbsPath.MakeCleanPath();
    }

    it = m_ShaderTypes.Insert(sShaderPath, ShaderData());
    it.Value().m_sShaderPath = sShaderPath;
    it.Value().m_sAbsShaderPath = sAbsPath;
    UpdateShaderType(it.Value());
  }

  return it.Value().m_pType;
}

void ezShaderTypeRegistry::UpdateShaderType(ShaderData& data)
{
  EZ_LOG_BLOCK("Updating Shader Parameters", data.m_sShaderPath.GetData());

  ezHybridArray<ezShaderParser::ParameterDefinition, 16> parameters;

  {
    ezFileStats Stats;
    bool bStat = ezOSFile::GetFileStats(data.m_sAbsShaderPath, Stats).Succeeded();

    ezFileReader file;
    if (!bStat || file.Open(data.m_sAbsShaderPath).Failed())
    {
      ezLog::Error("Can't update shader '{0}' type information, the file can't be opened.", data.m_sShaderPath);
      return;
    }

    ezShaderParser::ParseMaterialParameterSection(file, parameters);
    data.m_fileModifiedTime = Stats.m_LastModificationTime;
  }

  ezReflectedTypeDescriptor desc;
  desc.m_sTypeName = data.m_sShaderPath;
  desc.m_sPluginName = "ShaderTypes";
  desc.m_sParentTypeName = m_pBaseType->GetTypeName();;
  desc.m_Flags = ezTypeFlags::Phantom | ezTypeFlags::Class;
  desc.m_uiTypeSize = 0;
  desc.m_uiTypeVersion = 2;

  for (auto& parameter : parameters)
  {
    const ezRTTI* pType = GetType(parameter);
    if (pType == nullptr)
    {
      continue;
    }

    ezBitflags<ezPropertyFlags> flags = ezPropertyFlags::Phantom;
    if (pType->IsDerivedFrom<ezEnumBase>())
      flags |= ezPropertyFlags::IsEnum;
    if (pType->IsDerivedFrom<ezBitflagsBase>())
      flags |= ezPropertyFlags::Bitflags;
    if (ezReflectionUtils::IsBasicType(pType))
      flags |= ezPropertyFlags::StandardType;

    ezReflectedPropertyDescriptor propDesc(ezPropertyCategory::Member, parameter.m_sName, pType->GetTypeName(), flags);

    AddAttributes(parameter, pType, propDesc.m_Attributes);

    desc.m_Properties.PushBack(propDesc);
  }

  // Register and return the phantom type. If the type already exists this will update the type
  // and patch any existing instances of it so they should show up in the prop grid right away.
  ezPhantomRttiManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezShaderTypeRegistry::PhantomTypeRegistryEventHandler, this));
  {
    // We do not want to listen to type changes that we triggered ourselves.
    data.m_pType = ezPhantomRttiManager::RegisterType(desc);
  }
  ezPhantomRttiManager::s_Events.AddEventHandler(ezMakeDelegate(&ezShaderTypeRegistry::PhantomTypeRegistryEventHandler, this));
}

void ezShaderTypeRegistry::PhantomTypeRegistryEventHandler(const ezPhantomRttiManagerEvent& e)
{
  if (e.m_Type == ezPhantomRttiManagerEvent::Type::TypeAdded)
  {
    if (e.m_pChangedType->GetParentType() == m_pBaseType)
    {
      GetShaderType(e.m_pChangedType->GetTypeName());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>

/// \brief Changes the base class of all shader types to ezShaderTypeBase (version 1) and
/// sets their own version to 2.
class ezShaderTypePatch_1_2 : public ezGraphPatch
{
public:
  ezShaderTypePatch_1_2()
    : ezGraphPatch(nullptr, 2, ezGraphPatch::PatchType::GraphPatch) {}

  virtual void Patch(ezGraphPatchContext& context, ezAbstractObjectGraph* pGraph, ezAbstractObjectNode*) const override
  {
    ezString sDescTypeName = ezGetStaticRTTI<ezReflectedTypeDescriptor>()->GetTypeName();

    auto& nodes = pGraph->GetAllNodes();
    bool bNeedAddBaseClass = false;
    for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
    {
      ezAbstractObjectNode* pNode = it.Value();
      if (pNode->GetType() == sDescTypeName)
      {
        auto* pTypeProperty = pNode->FindProperty("TypeName");
        if (ezStringUtils::EndsWith(pTypeProperty->m_Value.Get<ezString>(), ".ezShader"))
        {
          auto* pTypeVersionProperty = pNode->FindProperty("TypeVersion");
          auto* pParentTypeProperty = pNode->FindProperty("ParentTypeName");
          if (pTypeVersionProperty->m_Value == 1)
          {
            pParentTypeProperty->m_Value = "ezShaderTypeBase";
            pTypeVersionProperty->m_Value = (ezUInt32)2;
            bNeedAddBaseClass = true;
          }
        }
      }
    }

    if (bNeedAddBaseClass)
    {
      ezRttiConverterContext context;
      ezRttiConverterWriter rttiConverter(pGraph, &context, true, true);

      ezReflectedTypeDescriptor desc;
      desc.m_sTypeName = "ezShaderTypeBase";
      desc.m_sPluginName = "ShaderTypes";
      desc.m_sParentTypeName = ezGetStaticRTTI<ezReflectedClass>()->GetTypeName();
      desc.m_Flags = ezTypeFlags::Phantom | ezTypeFlags::Abstract | ezTypeFlags::Class;
      desc.m_uiTypeSize = 0;
      desc.m_uiTypeVersion = 1;

      context.RegisterObject(ezUuid::StableUuidForString(desc.m_sTypeName.GetData()), ezGetStaticRTTI<ezReflectedTypeDescriptor>(), &desc);
      rttiConverter.AddObjectToGraph(ezGetStaticRTTI<ezReflectedTypeDescriptor>(), &desc);
    }
  }
};

ezShaderTypePatch_1_2 g_ezShaderTypePatch_1_2;

// TODO: Increase ezShaderTypeBase version to 2 and implement enum renames, see ezReflectedPropertyDescriptorPatch_1_2
class ezShaderBaseTypePatch_1_2 : public ezGraphPatch
{
public:
  ezShaderBaseTypePatch_1_2()
    : ezGraphPatch("ezShaderTypeBase", 2) {}

  static void FixEnumString(ezStringBuilder& sValue, const char* szName)
  {
    if (sValue.StartsWith(szName))
      sValue.Shrink(ezStringUtils::GetCharacterCount(szName), 0);

    if (sValue.StartsWith("::"))
      sValue.Shrink(2, 0);

    if (sValue.StartsWith(szName))
      sValue.Shrink(ezStringUtils::GetCharacterCount(szName), 0);

    if (sValue.StartsWith("_"))
      sValue.Shrink(1, 0);

    sValue.PrependFormat("{0}::{0}_", szName);
  }

  void FixEnum(ezAbstractObjectNode* pNode, const char* szEnum) const
  {
    if (ezAbstractObjectNode::Property* pProp = pNode->FindProperty(szEnum))
    {
      ezStringBuilder sValue = pProp->m_Value.Get<ezString>();
      FixEnumString(sValue, szEnum);
      pProp->m_Value = sValue.GetData();
    }
  }

  virtual void Patch(ezGraphPatchContext& context, ezAbstractObjectGraph* pGraph, ezAbstractObjectNode* pNode) const override
  {
    FixEnum(pNode, "SHADING_MODE");
    FixEnum(pNode, "BLEND_MODE");
    FixEnum(pNode, "RENDER_PASS");
  }
};

ezShaderBaseTypePatch_1_2 g_ezShaderBaseTypePatch_1_2;

