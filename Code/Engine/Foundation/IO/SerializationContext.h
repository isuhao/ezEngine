﻿#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Containers/Map.h>

/// \file

/// \brief A serialization context is used to (de-)serialize special types that require more complex logic than just moving a few bytes.
///
/// A serialization context can, for example, be used to keep track of which objects have already been serialized into a stream,
/// such that when they are written a second time to the stream, only a reference to the first one is actually stored.
/// Upon deserialization the context of course needs to resolve these references properly.
///
/// In general a serialization context only provides additional 'state' per stream and for a specific set of types. There can be any
/// number of serialization contexts active for the same stream, they are basically 'tacked on' from the outside, the stream itself
/// knows nothing of serialization contexts.
///
/// A serialization context is created by deriving from ezSerializationContext, passing the own class as the template argument.
///
/// In the derived class you have to implement two functions per type that the context is supposed to handle:\n
///   (virtual) void Read (ezStreamReader& stream, TYPE& type)\n
///   (virtual) void Write(ezStreamWriter& stream, const TYPE& type)\n
///
/// The macro EZ_ADD_SERIALIZATION_CONTEXT_OPERATORS() will then add the << and >> operators that redirect serialization of that type
/// to these Read/Write functions.
///
/// In these functions you can implement whatever complex logic is necessary to (de-)serialize the type and you can keep custom state.
///
/// When serializing to or from some stream, you need to create an instance of your serialization context and associate it with the
/// reader / writer stream. This should be done by the derived class implementation by calling RegisterReaderStream() and
/// RegisterWriterStream(). Once that is done, the static functions GetReaderContext() and GetWriterContext() give access to the
/// serialization context that is associated with a given stream. For details how this can be used, see the code in the 
/// EZ_ADD_SERIALIZATION_CONTEXT_OPERATORS() macro.
template<typename DERIVED>
class ezSerializationContext
{
public:

  /// \brief Returns the serialization context that is associated with the given stream (may be nullptr).
  ///
  /// As long as you use the EZ_ADD_SERIALIZATION_CONTEXT_OPERATORS() macro, it is typically not necessary to call this function directly.
  static DERIVED* GetReaderContext(ezStreamReader* pStream) // [tested]
  {
    return s_ActiveReaderContexts.GetValueOrDefault(pStream, nullptr);
  }

  /// \brief Returns the serialization context that is associated with the given stream (may be nullptr).
  ///
  /// As long as you use the EZ_ADD_SERIALIZATION_CONTEXT_OPERATORS() macro, it is typically not necessary to call this function directly.
  static DERIVED* GetWriterContext(ezStreamWriter* pStream) // [tested]
  {
    return s_ActiveWriterContexts.GetValueOrDefault(pStream, nullptr);
  }

protected:
  
  /// \brief Constructor.
  ezSerializationContext()
  {
    m_pReaderStream = nullptr;
    m_pWriterStream = nullptr;
  }

  /// \brief The destructor ensures that stream associations are reset.
  ~ezSerializationContext()
  {
    UnregisterReader();
    UnregisterWriter();
  }

  /// \brief The deriving class should call this to associate this serialization context with the given stream.
  ///
  /// Each context can only be associated with one reader and one writer stream.
  void RegisterReaderStream(ezStreamReader* pStream) // [tested]
  {
    if (pStream == m_pReaderStream)
      return;

    EZ_ASSERT_DEV(m_pReaderStream == nullptr, "A serialization context can only be associated to one reader and one writer stream.");

    m_pReaderStream = pStream;
    s_ActiveReaderContexts[pStream] = (DERIVED*) this;
  }

  /// \brief Resets the association with any previous reader stream.
  void UnregisterReader() // [tested]
  {
    if (m_pReaderStream == nullptr)
      return;

    s_ActiveReaderContexts.Remove(m_pReaderStream);
    m_pReaderStream = nullptr;
  }

  /// \brief The deriving class should call this to associate this serialization context with the given stream.
  ///
  /// Each context can only be associated with one reader and one writer stream.
  void RegisterWriterStream(ezStreamWriter* pStream) // [tested]
  {
    if (pStream == m_pWriterStream)
      return;

    EZ_ASSERT_DEV(m_pWriterStream == nullptr, "A serialization context can only be associated to one reader and one writer stream.");

    m_pWriterStream = pStream;
    s_ActiveWriterContexts[pStream] = (DERIVED*) this;
  }

  /// \brief Resets the association with any previous writer stream.
  void UnregisterWriter() // [tested]
  {
    if (m_pWriterStream == nullptr)
      return;

    s_ActiveWriterContexts.Remove(m_pWriterStream);
    m_pWriterStream = nullptr;
  }

private:
  ezStreamReader* m_pReaderStream;
  ezStreamWriter* m_pWriterStream;

  static ezMap<ezStreamReader*, DERIVED*> s_ActiveReaderContexts;
  static ezMap<ezStreamWriter*, DERIVED*> s_ActiveWriterContexts;
};


template<typename DERIVED>
ezMap<ezStreamReader*, DERIVED*> ezSerializationContext<DERIVED>::s_ActiveReaderContexts;

template<typename DERIVED>
ezMap<ezStreamWriter*, DERIVED*> ezSerializationContext<DERIVED>::s_ActiveWriterContexts;

/// \brief This macro adds serialization operators (<< and >>) for the given TYPE. The operators will redirect the serialization work
/// to the Read/Write functions in the given CONTEXT class (derived from ezSerializationContext).
///
/// This macro can be used for different types with the same CONTEXT, as long as the class provides the required Read/Write functions.
/// \sa ezSerializationContext
#define EZ_ADD_SERIALIZATION_CONTEXT_OPERATORS(CONTEXT, TYPE) \
  \
inline ezStreamWriter& operator<<(ezStreamWriter& stream, const TYPE type) \
{ \
  CONTEXT* pContext = CONTEXT::GetWriterContext(&stream); \
  EZ_ASSERT_DEV(pContext != nullptr, "No serialization context of type '" #CONTEXT "' that handles type '" #TYPE "' is available."); \
  \
  pContext->Write(stream, type); \
  \
  return stream; \
} \
\
inline ezStreamReader& operator>>(ezStreamReader& stream, TYPE type) \
{ \
  CONTEXT* pContext = CONTEXT::GetReaderContext(&stream); \
  EZ_ASSERT_DEV(pContext != nullptr, "No serialization context of type '" #CONTEXT "' that handles type '" #TYPE "' is available."); \
\
  pContext->Read(stream, type); \
\
  return stream; \
} \

/// \brief Add this to the derived ezSerializationContext class, if the Read and Write functions are not supposed to be public
#define EZ_MAKE_SERIALIZATION_CONTEXT_OPERATORS_FRIENDS(TYPE) \
  friend ezStreamWriter& operator<<(ezStreamWriter& stream, const TYPE type); \
  friend ezStreamReader& operator>>(ezStreamReader& stream, TYPE type);


