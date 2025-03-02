#ifndef SDI_HOOK_H
#define SDI_HOOK_H

/* Includeheader

        Name:           SDI_hook.h
        Versionstring:  $VER: SDI_hook.h 1.2 (18.10.2002)
        Author:         SDI
        Distribution:   PD
        Description:    defines to hide compiler specific hook stuff

 1.0   21.06.02 : based on the work made for freeciv and YAM with
        additional texts partly taken from YAM_hook.h changes made by Jens
        Langner, largely reworked the mechanism
 1.1   07.10.02 : added HOOKPROTONP and HOOKPROTONONP requested by Jens
 1.2   18.10.02 : reverted to old MorphOS-method for GCC
*/

/*
** This is PD (Public Domain). This means you can do with it whatever you want
** without any restrictions. I only ask you to tell me improvements, so I may
** fix the main line of this files as well.
**
** To keep confusion level low: When changing this file, please note it in
** above history list and indicate that the change was not made by myself
** (e.g.�add your name or nick name).
**
** Dirk St�cker <stoecker@epost.de>
*/

#include "SDI_compiler.h"

/*
** Hook macros to handle the creation of Hooks/Dispatchers for different
** Operating System versions.
** Currently AmigaOS and MorphOS is supported.
**
** For more information about hooks see include file <utility/hooks.h> or
** the relevant descriptions in utility.library autodocs.
**
** Example:
**
** Creates a hook with the name "TestHook" that calls a corresponding
** function "TestFunc" that will be called with a pointer "text"
** (in register A1) and returns a long.
**
** HOOKPROTONHNO(TestFunc, LONG, STRPTR text)
** {
**   Printf(text);
**   return 0;
** }
** MakeHook(TestHook, TestFunc);
**
** Every function that is created with HOOKPROTO* must have a MakeHook() or
** MakeStaticHook() to create the corresponding hook. Best is to call this
** directly after the hook function. This is required by the GCC macros.
**
** The naming convention for the Hook Prototype macros is as followed:
**
** HOOKPROTO[NH][NO][NP]
**           ^^  ^^  ^^
**      NoHook   |    NoParameter
**            NoObject
**
** So a plain HOOKPROTO() creates you a Hook function that requires
** 4 parameters, the "name" of the hookfunction, the "obj" in REG_A2,
** the "param" in REG_A1 and a "hook" in REG_A0. Usually you will always
** use NH, as the hook structure itself is nearly never required.
**
** The DISPATCHERPROTO macro is for MUI dispatcher functions. It gets the
** functionname as argument. To supply this function for use by MUI, use
** The ENTRY macro, which also gets the function name as argument.
*/

#if !defined(__MORPHOS__) || !defined(__GNUC__)
  #define HOOKPROTO(name, ret, obj, param) static SAVEDS ASM(ret)            \
    name(REG(a0, struct Hook *hook), REG(a2, obj), REG(a1, param))
  #define HOOKPROTONO(name, ret, param) static SAVEDS ASM(ret)               \
    name(REG(a0, struct Hook *hook), REG(a1, param))
  #define HOOKPROTONP(name, ret, obj) static SAVEDS ASM(ret)                 \
    name(REG(a0, struct Hook *hook), REG(a2, obj))
  #define HOOKPROTONONP(name, ret) static SAVEDS ASM(ret)                    \
    name(REG(a0, struct Hook *hook))
  #define HOOKPROTONH(name, ret, obj, param) static SAVEDS ASM(ret)          \
    name(REG(a2, obj), REG(a1, param))
  #define HOOKPROTONHNO(name, ret, param) static SAVEDS ASM(ret)             \
    name(REG(a1, param))
  #define HOOKPROTONHNP(name, ret, obj) static SAVEDS ASM(ret)               \
    name(REG(a2, obj))
  #define HOOKPROTONHNONP(name, ret) static SAVEDS ret name(void)
#endif

#ifdef __MORPHOS__
  #define SDI_TRAP_LIB 0xFF00 /* SDI prefix to reduce conflicts */

  struct SDI_EmulLibEntry
  {
    UWORD Trap;
    UWORD pad;
    APTR  Func;
  };

  #ifdef __GNUC__
    #include <emul/emulregs.h>

    #define HOOKPROTO(name, ret, obj, param) static ret name(void)           \
      { struct Hook *hook = REG_A0; obj = REG_A2; param = REG_A1;
    #define HOOKPROTONO(name, ret, param) static ret name(void)              \
      { struct Hook *hook = REG_A0; param = REG_A1;
    #define HOOKPROTONP(name, ret, obj) static ret name(void)                \
      { struct Hook *hook = REG_A0; obj = REG_A2;
    #define HOOKPROTONONP(name, ret) static ret name(void)                   \
      { struct Hook *hook = REG_A0;
    #define HOOKPROTONH(name, ret, obj, param) static ret name(void)         \
      { obj = REG_A2; param = REG_A1;
    #define HOOKPROTONHNO(name, ret, param) static ret name(void)            \
      { param = REG_A1;
    #define HOOKPROTONHNP(name, ret, obj) static ret name(void)              \
      { obj = REG_A2;
    #define HOOKPROTONHNONP(name, ret) static ret name(void) {
    #define MakeHook(hookname, funcname)                                     \
      } static const struct SDI_EmulLibEntry Gate_##funcname =               \
      {SDI_TRAP_LIB, 0, (void(*)()) funcname};                               \
      struct Hook hookname = {{NULL, NULL}, (HOOKFUNC)&Gate_##funcname,      \
      NULL, NULL}
    #define MakeStaticHook(hookname, funcname)                               \
      } static const struct SDI_EmulLibEntry Gate_##funcname =               \
      {SDI_TRAP_LIB, 0, (void(*)()) funcname};                               \
      static struct Hook hookname = {{NULL, NULL},                           \
      (HOOKFUNC)&Gate_##funcname, NULL, NULL}
    #define DISPATCHERPROTO(name)                                            \
      struct IClass;                                                         \
      static ULONG name(struct IClass * cl, Object * obj, Msg msg);          \
      static ULONG Trampoline_##name(void) {return name((struct IClass *)    \
      REG_A0, (Object *) REG_A2, (Msg) REG_A1);}                             \
      static const struct SDI_EmulLibEntry Gate_##name = {SDI_TRAP_LIB, 0,   \
      (void(*)())Trampoline_##name};                                         \
      static ULONG name(struct IClass * cl, Object * obj, Msg msg)
  #else
    #define MakeHook(hookname, funcname)                                     \
      static const struct SDI_EmulLibEntry Gate_##funcname = {SDI_TRAP_LIB,  \
      0, (APTR) funcname};                                                   \
      struct Hook hookname = {{NULL, NULL},                                  \
      (HOOKFUNC)&Gate_##funcname, NULL, NULL}
    #define MakeStaticHook(hookname, funcname)                               \
      static const struct SDI_EmulLibEntry Gate_##funcname = {SDI_TRAP_LIB,  \
      0, (APTR) funcname};                                                   \
      static struct Hook hookname = {{NULL, NULL},                           \
      (HOOKFUNC)&Gate_##funcname, NULL, NULL}
    #define DISPATCHERPROTO(name)                                            \
      struct IClass;                                                         \
      static ASM(ULONG) name(REG(a0,                                         \
      struct IClass * cl), REG(a2, Object * obj), REG(a1, Msg msg));         \
      static const struct SDI_EmulLibEntry Gate_##name = {SDI_TRAP_LIB, 0,   \
      (APTR)name};                                                           \
      static ASM(ULONG) name(REG(a0,                                         \
      struct IClass * cl), REG(a2, Object * obj), REG(a1, Msg msg))
  #endif

  #define ENTRY(func) (APTR)&Gate_##func
#else
  #define DISPATCHERPROTO(name) static SAVEDS ASM(ULONG) name(REG(a0,        \
    struct IClass * cl), REG(a2, Object * obj), REG(a1, Msg msg))
  #define ENTRY(func) (APTR)func

  #define MakeHook(hookname, funcname) struct Hook hookname = {{NULL, NULL}, \
    (HOOKFUNC)funcname, NULL, NULL}
  #define MakeStaticHook(hookname, funcname) static struct Hook hookname =   \
    {{NULL, NULL}, (HOOKFUNC)funcname, NULL, NULL}
#endif

#define InitHook(hook, orighook, data) ((hook)->h_Entry = (orighook).h_Entry,\
  (hook)->h_Data = (APTR)(data))

#endif /* SDI_HOOK_H */
