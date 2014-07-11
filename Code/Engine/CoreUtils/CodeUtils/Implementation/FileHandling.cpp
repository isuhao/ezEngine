#include <CoreUtils/PCH.h>
#include <CoreUtils/CodeUtils/Preprocessor.h>
#include <Foundation/Utilities/ConversionUtils.h>

ezMap<ezString, ezTokenizer>::ConstIterator ezTokenizedFileCache::Lookup(const ezString& sFileName) const
{
  EZ_LOCK(m_Mutex);
  auto it = m_Cache.Find(sFileName);
  return it;
}

void ezTokenizedFileCache::Remove(const ezString& sFileName)
{
  EZ_LOCK(m_Mutex);
  m_Cache.Erase(sFileName);
}

void ezTokenizedFileCache::Clear()
{
  EZ_LOCK(m_Mutex);
  m_Cache.Clear();
}

static void SkipWhitespace(ezDeque<ezToken>& Tokens, ezUInt32& uiCurToken)
{
  while (uiCurToken < Tokens.GetCount() && 
          (Tokens[uiCurToken].m_iType == ezTokenType::BlockComment ||
           Tokens[uiCurToken].m_iType == ezTokenType::LineComment ||
           Tokens[uiCurToken].m_iType == ezTokenType::Newline ||
           Tokens[uiCurToken].m_iType == ezTokenType::Whitespace))
    ++uiCurToken;
}

const ezTokenizer* ezTokenizedFileCache::Tokenize(const ezString& sFileName, const ezDynamicArray<ezUInt8>& FileContent, ezLogInterface* pLog)
{
  EZ_LOCK(m_Mutex);
  ezTokenizer* pTokenizer = &m_Cache[sFileName];
  pTokenizer->Tokenize(FileContent, pLog);

  ezDeque<ezToken>& Tokens = pTokenizer->GetTokens();

  ezHashedString sFile;
  sFile.Assign(sFileName.GetData());

  ezInt32 iLineOffset = 0;

  for (ezUInt32 i = 0; i + 1 < Tokens.GetCount(); ++i)
  {
    const ezUInt32 uiCurLine = Tokens[i].m_uiLine;

    Tokens[i].m_File = sFile;
    Tokens[i].m_uiLine += iLineOffset;

    if (Tokens[i].m_iType == ezTokenType::NonIdentifier &&
        ezString(Tokens[i].m_DataView) == "#")
    {
      ezUInt32 uiNext = i + 1;

      SkipWhitespace(Tokens, uiNext);

      if (uiNext < Tokens.GetCount() && 
          Tokens[uiNext].m_iType == ezTokenType::Identifier &&
          ezString(Tokens[uiNext].m_DataView) == "line")
      {
        ++uiNext;
        SkipWhitespace(Tokens, uiNext);

        if (uiNext < Tokens.GetCount() && 
            Tokens[uiNext].m_iType == ezTokenType::Identifier)
        {
          ezInt32 iNextLine = 0;

          const ezString sNumber = Tokens[uiNext].m_DataView;
          if (ezConversionUtils::StringToInt(sNumber.GetData(), iNextLine).Succeeded())
          {
            iLineOffset = (iNextLine - uiCurLine) - 1;

            ++uiNext;
            SkipWhitespace(Tokens, uiNext);

            if (uiNext < Tokens.GetCount())
            {
              if (Tokens[uiNext].m_iType == ezTokenType::String1)
              {
                ezStringBuilder sFileName = Tokens[uiNext].m_DataView;
                sFileName.Shrink(1, 1); // remove surrounding "

                sFile.Assign(sFileName.GetData());
              }
            }
          }
        }
      }
    }
  }

  return pTokenizer;
}


void ezPreprocessor::SetLogInterface(ezLogInterface* pLog)
{
  m_pLog = pLog;
}

void ezPreprocessor::SetFileCallbacks(FileOpenCB OpenAbsFileCB, FileLocatorCB LocateAbsFileCB)
{
  m_FileOpenCallback = OpenAbsFileCB;
  m_FileLocatorCallback = LocateAbsFileCB;
}

ezResult ezPreprocessor::OpenFile(const char* szFile, const ezTokenizer** pTokenizer)
{
  EZ_ASSERT(m_FileOpenCallback != nullptr, "OpenFile callback has not been set");
  EZ_ASSERT(m_FileLocatorCallback != nullptr, "File locator callback has not been set");

  *pTokenizer = nullptr;

  auto it = m_pUsedFileCache->Lookup(szFile);

  if (it.IsValid())
  {
    *pTokenizer = &it.Value();
    return EZ_SUCCESS;
  }

  ezDynamicArray<ezUInt8> Content;
  if (m_FileOpenCallback(szFile, Content).Failed())
  {
    ezLog::Error(m_pLog, "Could not open file '%s'", szFile);
    return EZ_FAILURE;
  }

  *pTokenizer = m_pUsedFileCache->Tokenize(szFile, Content, m_pLog);


  return EZ_SUCCESS;
}


ezResult ezPreprocessor::HandleInclude(const TokenStream& Tokens, ezUInt32 uiCurToken, ezUInt32 uiDirectiveToken, TokenStream& TokenOutput)
{
  EZ_ASSERT(m_FileLocatorCallback != nullptr, "File locator callback has not been set");

  SkipWhitespace(Tokens, uiCurToken);

  ezStringBuilder sPath;

  IncludeType IncType = IncludeType::GlobalInclude;

  ezUInt32 uiAccepted;
  if (Accept(Tokens, uiCurToken, ezTokenType::String1, &uiAccepted))
  {
    IncType = IncludeType::RelativeInclude;
    sPath = Tokens[uiAccepted]->m_DataView;
    sPath.Shrink(1, 1); // remove " at start and end
  }
  else
  {
    // in global include paths (ie. <bla/blub.h>) we need to handle line comments special
    // because a path with two slashes will be a comment token, although it could be a valid path
    // so we concatenate just everything and then make sure it ends with a >

    if (Expect(Tokens, uiCurToken, "<", &uiAccepted).Failed())
      return EZ_FAILURE;

    TokenStream PathTokens;

    while (uiCurToken < Tokens.GetCount())
    {
      if (Tokens[uiCurToken]->m_iType == ezTokenType::Newline)
      {
        ++uiCurToken;
        continue;
      }

      PathTokens.PushBack(Tokens[uiCurToken]);
      ++uiCurToken;
    }

    CombineTokensToString(PathTokens, 0, sPath);

    // remove all whitespace at the end (this could be part of a comment, so not tokenized as whitespace)
    while (sPath.EndsWith(" ") || sPath.EndsWith("\t"))
      sPath.Shrink(0, 1);

    // there must always be a > at the end, although it could be a separate token or part of a comment
    // so we check the string, instead of the tokens
    if (sPath.EndsWith(">"))
      sPath.Shrink(0, 1);
    else
    {
      PP_LOG(Error, "Invalid include path '%s'", Tokens[uiAccepted], sPath.GetData());
      return EZ_FAILURE;
    }
  }

  if (ExpectEndOfLine(Tokens, uiCurToken).Failed())
  {
    PP_LOG0(Error, "Expected end-of-line", Tokens[uiCurToken]);
    return EZ_FAILURE;
  }

  EZ_ASSERT(!m_sCurrentFileStack.IsEmpty(), "Implementation error.");

  ezString sOtherFile;

  if (m_FileLocatorCallback(m_sCurrentFileStack.PeekBack().m_sFileName.GetData(), sPath.GetData(), IncType, sOtherFile).Failed())
  {
    PP_LOG(Error, "#include file '%s' could not be located", Tokens[uiAccepted], sPath.GetData());
    return EZ_FAILURE;
  }

  // if this has been included before, and contains a #pragma once, do not include it again
  if (m_PragmaOnce.Find(sOtherFile.GetData()).IsValid())
    return EZ_SUCCESS;

  if (ProcessFile(sOtherFile.GetData(), TokenOutput).Failed())
    return EZ_FAILURE;

  if (uiCurToken < Tokens.GetCount() && (Tokens[uiCurToken]->m_iType == ezTokenType::Newline || Tokens[uiCurToken]->m_iType == ezTokenType::EndOfFile))
    TokenOutput.PushBack(Tokens[uiCurToken]);

  return EZ_SUCCESS;
}
