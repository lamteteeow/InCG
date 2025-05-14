#pragma once

// ----------------------------------------------------------------------------
//  OS specific definitions    (based on AntTweakBar)
// ----------------------------------------------------------------------------

#if (defined(_WIN32) || defined(_WIN64)) && !defined(TW_STATIC)
#   define INCG_CALL          __stdcall
#   define INCG_CDECL_CALL    __cdecl
#   define INCG_EXPORT_API    __declspec(dllexport)
#   define INCG_IMPORT_API    __declspec(dllimport)
#else
#   define INCG_CALL
#   define INCG_CDECL_CALL
#   define INCG_EXPORT_API
#   define INCG_IMPORT_API
#endif

#if defined INCG_EXPORTS
#   define INCG_API INCG_EXPORT_API
#elif defined INCG_STATIC
#   define INCG_API
#   if defined(_MSC_VER) && !defined(INCG_NO_LIB_PRAGMA)
#       ifdef _WIN64
#           pragma comment(lib, "INCGStatic64")
#       else
#           pragma comment(lib, "INCGStatic")
#       endif
#   endif
#else
#   define INCG_API INCG_IMPORT_API
#   if defined(_MSC_VER) && !defined(INCG_NO_LIB_PRAGMA)
#       ifdef _WIN64
#           pragma comment(lib, "INCG64")
#       else
#           pragma comment(lib, "INCG")
#       endif
#   endif
#endif
