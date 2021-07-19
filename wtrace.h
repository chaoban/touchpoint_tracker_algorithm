/*++
    Copyright (c) Silicon Integrated Systems Corporation.  All rights reserved.

    Module Name:
        wtrace.h

    Abstract: 
        contains placeholders for useful trace utilities

--*/

#ifndef _WTRACE_H
#define _WTRACE_H


//
// Key definitions
//



//
// Constants
//
//#ifdef DEBUG
#define API             "API"
#define Func            "Func"

// Msg filter - general
#define MSGFLTR_NONUSE          DbgMsgFilter
#define MSGFLTR_DEFAULT         (MSGTYPE_ASSERT | MSGTYPE_ERR | MSGTYPE_WARN)
#define MSGFLTR_GENERAL         ((MSGTYPE_ASSERT | MSGTYPE_ERR | MSGTYPE_WARN) ||   \
                                (MSGMDL_OEMTP | MSGMDL_SISTP | MSGMDL_SISVALID |    \
                                MSGMDL_SISTRANS | MSGMDL_OEMINBUF))
#define MSGFLTR_ALLTYPE         (MSGTYPE_ASSERT | MSGTYPE_ENTER | MSGTYPE_EXIT |    \
                                MSGTYPE_ERR | MSGTYPE_FATAL | MSGTYPE_WARN | MSGTYPE_INFO)
// Msg filter - path
#define MSGFLTR_PATH_INPUTDATA  ((ULONG)(MSGFLTR_ALLTYPE | MSGTYPE_MODULE | MSGMDL_INPUTDATA))
#define MSGFLTR_INPUTDATA       MSGFLTR_PATH_INPUTDATA
// Msg filter - user
#define MSGFLTR_USER_USER0      (MSGFLTR_ALLTYPE | MSGTYPE_MODULE | MSGMDL_USER0)
#define MSGFLTR_USER0           MSGFLTR_USER_USER0
#define MSGFLTR_USER_USER1      (MSGFLTR_ALLTYPE | MSGTYPE_MODULE | MSGMDL_USER1)
#define MSGFLTR_USER1           MSGFLTR_USER_USER1
#define MSGFLTR_USER_USER2      (MSGFLTR_ALLTYPE | MSGTYPE_MODULE | MSGMDL_USER2)
#define MSGFLTR_USER2           MSGFLTR_USER_USER2
#define MSGFLTR_USER_USER3      (MSGFLTR_ALLTYPE | MSGTYPE_MODULE | MSGMDL_USER3)
#define MSGFLTR_USER3           MSGFLTR_USER_USER3

// Msg file module
#define MODULE_HIDTP            0
#define MODULE_PNP              1
#define MODULE_IOCTL            2
#define MODULE_SERIAL           3
#define MODULE_OEMTP            4
#define MODULE_SISTP            5
#define MODULE_SISINPUT         6
#define MODULE_SISVALID         7
#define MODULE_SISTRANS         8
#define MODULE_OEMINBUF         9

#define MSGMDL_MASK             0xFFFF
#define MSGMDL_HIDTP            (0x1 << MODULE_HIDTP)
#define MSGMDL_PNP              (0x1 << MODULE_PNP)
#define MSGMDL_IOCTL            (0x1 << MODULE_IOCTL)
#define MSGMDL_SERIAL           (0x1 << MODULE_SERIAL)
#define MSGMDL_OEMTP            (0x1 << MODULE_OEMTP)
#define MSGMDL_SISTP            (0x1 << MODULE_SISTP)
#define MSGMDL_SISINPUT         (0x1 << MODULE_SISVALID)
#define MSGMDL_SISVALID         (0x1 << MODULE_SISVALID)
#define MSGMDL_SISTRANS         (0x1 << MODULE_SISTRANS)
#define MSGMDL_OEMINBUF         (0x1 << MODULE_OEMINBUF)

// Msg path module
#define MSGMDL_PATHSHIFT        12
#define MSGMDL_INPUTDATA        (0x1 << (MSGMDL_PATHSHIFT))

// Msg user test module
#define MSGMDL_USERSHIT         16
#define MSGMDL_USER0            (0x1 << (MSGMDL_USERSHIFT))
#define MSGMDL_USER1            (0x1 << (MSGMDL_USERSHIFT + 1))
#define MSGMDL_USER2            (0x1 << (MSGMDL_USERSHIFT + 2))
#define MSGMDL_USER3            (0x1 << (MSGMDL_USERSHIFT + 3))

// Msg type
#define MSGTYPE_SHIFT           24
#define MSGTYPE_ASSERT          (0x1 << (0 + MSGTYPE_SHIFT))
#define MSGTYPE_ENTER           (0x1 << (1 + MSGTYPE_SHIFT))
#define MSGTYPE_EXIT            (0x1 << (2 + MSGTYPE_SHIFT))
#define MSGTYPE_FATAL           (0x1 << (3 + MSGTYPE_SHIFT))
#define MSGTYPE_ERR             (0x1 << (4 + MSGTYPE_SHIFT))
#define MSGTYPE_WARN            (0x1 << (5 + MSGTYPE_SHIFT))
#define MSGTYPE_INFO            (0x1 << (6 + MSGTYPE_SHIFT))
#define MSGTYPE_MODULE          (0x1 << (7 + MSGTYPE_SHIFT))
//#endif


//
// Macros
//
#ifdef DEBUG
  #define DBGMSG(x)             KdPrint(x)
  #define DBGBREAK              DbgBreakPoint
#else
  #define DBGMSG(x)
  #define DBGBREAK
#endif

#ifdef DEBUG

#define IS_DBGTYPE(t)           (t & DbgMsgFilter)
#define IS_DBGMODULE()          (MSGMDL_ID & DbgMsgFilter)

#define TAssert(x)                                  \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_ASSERT))                 \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            if (!(x))                               \
            {                                       \
                DBGMSG(("Assert: x.\n"));           \
                DBGBREAK();                         \
            }                                       \
        }                                           \
    }                                               \
}

#define TEnter(n, p)                                \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_ENTER))                  \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(("[%s] ", n));                   \
            DBGMSG(("%s", __FUNCTION__));           \
            DBGMSG(p);                              \
            DBGMSG(("\n"));                         \
        }                                           \
    }                                               \
}

#define TExit(n, p)                                 \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_EXIT))                   \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(("[%s] ", n));                   \
            DBGMSG(("return"));                     \
            DBGMSG(p);                              \
            DBGMSG(("\n"));                         \
        }                                           \
    }                                               \
}

#define TFatal(x)                                   \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_FATAL))                  \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(("Fatal Error: "));              \
            DBGMSG(x);                              \
            DBGMSG(("\n"));                         \
        }                                           \
    }                                               \
}

#define TErr(x)                                     \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_ERR))                    \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(("Error: "));                    \
            DBGMSG(x);                              \
            DBGMSG(("\n"));                         \
        }                                           \
    }                                               \
}

#define TWarn(x)                                    \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_WARN))                    \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(("Warn: "));                     \
            DBGMSG(x);                              \
            DBGMSG(("\n"));                         \
        }                                           \
    }                                               \
}

#define TInfo(x)                                    \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_INFO))                   \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(("Info: "));                     \
            DBGMSG(x);                              \
            DBGMSG(("\n"));                         \
        }                                           \
    }                                               \
}

#define TPrint(x)                                   \
{                                                   \
    if (IS_DBGTYPE(MSGTYPE_INFO))                   \
    {                                               \
        if (!IS_DBGTYPE(MSGTYPE_MODULE) ||          \
            (IS_DBGTYPE(MSGTYPE_MODULE) &&          \
            IS_DBGMODULE()))                        \
        {                                           \
            DBGMSG(x);                              \
        }                                           \
    }                                               \
}

#else
#define TAssert(x)
#define TEnter(n, p)
#define TExit(n, p)
#define TFatal(x)
#define TErr(x)
#define TWarn(x)
#define TInfo(x)
#define TPrint(x)
#endif

#ifdef DEBUG

#define MTAssert(f, x)                      \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TAssert(x);                             \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTEnter(f, n, p)                    \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TEnter(n, p);                           \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTExit(f, n, p)                     \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TExit(n, p);                            \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTFatal(f, x)                       \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TFatal(x);                              \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTErr(f, x)                         \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TErr(x);                                \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTWarn(f, x)                        \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TWarn(x);                               \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTInfo(f, x)                        \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TInfo(x);                               \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#define MTPrint(f, x)                       \
{                                           \
    DbgMsgFilterTemp = DbgMsgFilter;        \
    if ((DbgMsgFilter & f) & MSGMDL_MASK)   \
    {                                       \
        DbgMsgFilter = (f | MSGMDL_ID);     \
    }                                       \
    TPrint(x);                              \
    DbgMsgFilter = DbgMsgFilterTemp;        \
}

#else
#define MTAssert(f, x)
#define MTEnter(f, n, p)
#define MTExit(f, n, p)
#define MTFatal(f, x)
#define MTErr(f, x)
#define MTWarn(f, x)
#define MTInfo(f, x)
#define MTPrint(f, x)
#endif


//
// Type Definitions
//


//
// Function prototypes
//
#ifdef DEBUG
VOID INTERNAL
PrintMatrix(
    __in     ULONG MsgFilter,
    __in     PUCHAR Buffer,
    __in     ULONG Length,
    __in     ULONG Align
    );
#else
#define PrintMatrix(f, b, l, a)
#endif


//
// Global Data Declarations
//
#ifdef DEBUG
extern ULONG DbgMsgFilter;
extern ULONG DbgMsgFilterTemp;
#endif


#endif  //ifndef _WTRACE_H
