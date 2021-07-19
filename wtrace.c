/*++
    Copyright (c) Silicon Integrated Systems Corporation

    Module Name:
        wtrace.c

    Abstract: contains placeholders for useful trace utilities.

    Environment:
        Kernel mode

--*/

//#include "pch.h" //LINUX

#define MODULE_ID                       0xff
#define MSGMDL_ID                       5

//
// Key definitions
//


//
// Constants
//


//
// Macros
//


//
// Type definitions
//


//
// Extern function prototype.
//


//
// Local function prototype.
//


//
// Global variable definitions
//
#ifdef DEBUG
ULONG DbgMsgFilter = MSGFLTR_DEFAULT;
ULONG DbgMsgFilterTemp = MSGFLTR_DEFAULT;
#endif


//
// special print format functions
//

#ifdef DEBUG
VOID INTERNAL
PrintMatrix(
    __in     ULONG MsgFilter,
    __in     PUCHAR Buffer,
    __in     ULONG Length,
    __in     ULONG Align
    )
{
    ULONG i;
    UNREFERENCED_PARAMETER(MsgFilter);
//    UNREFERENCED_PARAMETER(MsgFilter);

    TInfo(("Address: %p, Length: %d.", Buffer, Length));
    for (i = 0; i < Length; i++)
    {
        TPrint(("%2x ", Buffer[i]));
        if (!((i + 1) % Align))
        {
            TPrint(("\n"));
        }
    }
    TPrint(("\n"));

    return;
}
#endif
