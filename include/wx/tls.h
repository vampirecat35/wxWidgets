///////////////////////////////////////////////////////////////////////////////
// Name:        wx/tls.h
// Purpose:     Implementation of thread local storage (obsolete)
// Author:      Vadim Zeitlin
// Created:     2008-08-08
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TLS_H_
#define _WX_TLS_H_

#include "wx/defs.h"

// ----------------------------------------------------------------------------
// check for compiler support of thread-specific variables
// ----------------------------------------------------------------------------

#if wxUSE_THREADS && defined(__RETRO68__)

// Retro68 compiler doesn't support thread_local and the (cooperative) Thread
// Manager threads used by pthreads-retro68 wouldn't switch the thread pointer
// anyhow, so use pthread keys for the thread-specific values instead. Note
// that wxTHREAD_SPECIFIC_DECL and wxHAS_COMPILER_TLS are not defined in this
// case and wxTLS_XXX() macros must be used instead.

#include <pthread.h>

template <typename T>
class wxTlsValue
{
public:
    typedef T ValueType;

    wxTlsValue()
    {
        m_ok = pthread_key_create(&m_key, &wxTlsValue::DeleteValue) == 0;
    }

    ~wxTlsValue()
    {
        if ( !m_ok )
            return;

        // The values of the other threads are deleted when they exit, but
        // the one of this (normally main) thread needs to be deleted now.
        DeleteValue(pthread_getspecific(m_key));
        pthread_key_delete(m_key);
    }

    // Return the value for the current thread, creating it if necessary.
    T *Get()
    {
        if ( !m_ok )
        {
            // This shouldn't happen, but fall back to a shared value rather
            // than crashing if we ran out of keys.
            static T s_fallback{};
            return &s_fallback;
        }

        void* value = pthread_getspecific(m_key);
        if ( !value )
        {
            value = new T{};
            pthread_setspecific(m_key, value);
        }

        return static_cast<T *>(value);
    }

    T *operator->() { return Get(); }
    T& operator*() { return *Get(); }

private:
    static void DeleteValue(void* value)
    {
        delete static_cast<T *>(value);
    }

    pthread_key_t m_key;
    bool m_ok;

    wxTlsValue(const wxTlsValue&) = delete;
    wxTlsValue& operator=(const wxTlsValue&) = delete;
};

#define wxTLS_TYPE(T) wxTlsValue<T>
#define wxTLS_TYPE_REF(T) wxTlsValue<T>&
#define wxTLS_PTR(var) ((var).Get())
#define wxTLS_VALUE(var) (*(var).Get())

#else // !Retro68 with threads

// when not using threads at all, there is no need for thread-specific
// values to be really thread-specific
#if !wxUSE_THREADS
    #define wxTHREAD_SPECIFIC_DECL
#else
    #define wxTHREAD_SPECIFIC_DECL thread_local
#endif

// ----------------------------------------------------------------------------
// define wxTLS_TYPE()
// ----------------------------------------------------------------------------

// All these preprocessor symbols are obsolete and defined only for
// compatibility: C++11 compilers always support variables with thread storage,
// so you should just always use thread_local directly in the new code instead.
// The exception is Retro68 (see above), where they're required when using
// threads, which is why they're still used in wx itself.
#define wxHAS_COMPILER_TLS

#define wxTLS_TYPE(T) wxTHREAD_SPECIFIC_DECL T
#define wxTLS_TYPE_REF(T) T&
#define wxTLS_PTR(var) (&(var))
#define wxTLS_VALUE(var) (var)

#endif // Retro68 with threads/others

#endif // _WX_TLS_H_

