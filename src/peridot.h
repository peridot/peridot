#ifndef _PERIDOT_H
#define _PERIDOT_H

// TODO: for when the language is more mature.
/* #include "platform.h" */
// TODO: If we ever need to mess around with dll exports.
/*
#ifdef LB_PLATFORM_WINDOWS
#define LB_DLLEXPORT __declspec(dllexport)
#else
#define LB_DLLEXPORT
#endif // LB_PLATFORM_WINDOWS
*/

// Computed Goto speeds up the VM dispatch loop but is not available under Visual Studio.
// TODO: The macro is here but the support isn't, support this when the VM is a bit more mature.
#ifndef PVM_COMPUTED_GOTO
  #ifdef _MSC_VER
    #define PVM_COMPUTED_GOTO 0
  #else
    #define PVM_COMPUTED_GOTO 1
  #endif // _MSC_VER
#endif // PVM_COMPUTED_GOTO

#ifdef __cplusplus
#define PERIDOT_EXTERN_C_BEGIN extern "C" {
#define PERIDOT_EXTERN_C_END }
#else
#define PERIDOT_EXTERN_C_BEGIN
#define PERIDOT_EXTERN_C_END
#endif // __cplusplus

// Visual Studio does not support "inline" when compiling as C.
#if defined(_MSC_VER) && !defined(__cplusplus)
#define PD_INLINE _inline
#else
#define PD_INLINE inline
#endif

#endif // _PERIDOT_H
