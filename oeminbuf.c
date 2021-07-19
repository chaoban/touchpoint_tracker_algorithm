/*++
    Copyright (c) Silicon Integrated Systems Corporation

    Module Name:
        sisinbuf.c

    Abstract:
        Contains OEM common functions for process input and resync buffer.

    Environment:
        Kernel mode

--*/

#include "sisinput.h"
//#include "common.h"
#define MODULE_ID                       MODULE_OEMINBUF
#define MSGMDL_ID                       MSGMDL_OEMINBUF


//
// Key definitions
//


//
// Constants
//

//
// Macro
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
VOID INTERNAL
PrintResync(
    __in     PRESYNC_BUFFER ResyncBuffer
    );


//
// Global variable definitions
//


//
// Input buffer operate functions (for extern)
//

PUCHAR INTERNAL
OemSetNextInputBuff(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp,
    __in     ULONG          BytesToRead
    )
{
    ULONG i, EmptyIndex = 0;
    BOOLEAN FirstIndex = FALSE;

    TEnter(Func, ("(InputBuffers=%p,Irp=%p,BytesToRead=%d)", InputBuffers, Irp, BytesToRead));

    // find nonuse
    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (!(InputBuffers->InputBuffer[i].Stamp || 
            InputBuffers->InputBuffer[i].Irp ||
            InputBuffers->InputBuffer[i].BytesToRead ||
            FirstIndex))
        {
            EmptyIndex = i;
            FirstIndex = TRUE;
        }
        if (InputBuffers->Stamp == InputBuffers->InputBuffer[i].Stamp &&
            InputBuffers->InputBuffer[i].Irp)
        {
            TInfo(("Info: (OemSetNextInputBuff) match buffer Stamp, Irp (%d, %p)", InputBuffers->Stamp, Irp));
        }
        else if (InputBuffers->InputBuffer[i].Stamp || 
                 InputBuffers->InputBuffer[i].Irp ||
                 InputBuffers->InputBuffer[i].BytesToRead)
        {
            TInfo(("(OemSetNextInputBuff) no match buffer Stamp, Irp %d (%d, %p)", InputBuffers->Stamp, InputBuffers->InputBuffer[i].Stamp, InputBuffers->InputBuffer[i].Irp));
        }
    }

    if (!FirstIndex)
    {
        TErr(("(OemSetNextInputBuff) all buffer isn't empty"));
        TPrint(("Stamp, Irp (%d, %p) ", InputBuffers->Stamp, Irp));
        for (i = 0; i < NUM_INPUT_SWAP; i++)
        {
            TPrint(("(%d, %p) ", InputBuffers->InputBuffer[i].Stamp, InputBuffers->InputBuffer[i].Irp));
        }
        TPrint(("\n"));
    }

    i = EmptyIndex;
    MTInfo(MSGFLTR_INPUTDATA, ("(OemSetNextInputBuff) Set index %d.", i));

    if (InputBuffers->InputBuffer[i].Stamp || 
        InputBuffers->InputBuffer[i].Irp ||
        InputBuffers->InputBuffer[i].BytesToRead)
    {
        TErr(("(OemSetNextInputBuff) nonuse buffer isn't empty %d %p %d.", InputBuffers->InputBuffer[i].Stamp, InputBuffers->InputBuffer[i].Irp, InputBuffers->InputBuffer[i].BytesToRead));
    }

    // set next nonuse buffer to inuse
    InputBuffers->Stamp++;
    InputBuffers->InputBuffer[i].Stamp = InputBuffers->Stamp;
    InputBuffers->InputBuffer[i].Irp = Irp;
    InputBuffers->InputBuffer[i].BytesToRead = BytesToRead;

    TExit(Func, ("=%p", InputBuffers->InputBuffer[i].RawInput));
    return InputBuffers->InputBuffer[i].RawInput;
}       //OemSetNextInputBuff

ULONG INTERNAL
OemGetInputBuffIndex(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
    ULONG i;

    TEnter(Func, ("(InputBuffers=%p,Irp=%p)", InputBuffers, Irp));

    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (InputBuffers->InputBuffer[i].Irp == Irp)
        {
            break;
        }
    }

    if (i == NUM_INPUT_SWAP)
    {
        i = 0;
        TErr(("(OemGetInputBuffIndex) no match buffer.")); 
    }

    TExit(Func, ("=%d", i));
    return i;
}       //OemGetInputBuffIndex

PUCHAR INTERNAL
OemGetInputBuff(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
    ULONG i = OemGetInputBuffIndex(InputBuffers, Irp);

    TEnter(Func, ("(InputBuffers=%p,Irp=%p)", InputBuffers, Irp));

    TExit(Func, ("=%p", InputBuffers->InputBuffer[i].RawInput));
    return InputBuffers->InputBuffer[i].RawInput;
}       //OemGetInputBuff

NTSTATUS INTERNAL
OemResetInputBuff(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    TEnter(Func, ("(InputBuffers=%p,Irp=%p)", InputBuffers, Irp));

    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (InputBuffers->InputBuffer[i].Irp == Irp)
        {
            InputBuffers->InputBuffer[i].Stamp = 0;
            InputBuffers->InputBuffer[i].Irp = 0;
            InputBuffers->InputBuffer[i].BytesToRead = 0;
            break;
        }
    }

    if (i == NUM_INPUT_SWAP)
    {
        status = STATUS_DATA_ERROR;
        TErr(("(OemResetInputBuff) no match buffer."));
    }

    TExit(Func, ("=%x", status));
    return status;
}       //OemResetInputBuff

NTSTATUS INTERNAL
OemUpdateInputReadLen(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp,
    __in     ULONG          BytesToRead
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    TEnter(Func, ("(InputBuffers=%p,Irp=%p,BytesToRead=%d)", InputBuffers, Irp, BytesToRead));

    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (InputBuffers->InputBuffer[i].Irp == Irp)
        {
            InputBuffers->InputBuffer[i].BytesToRead = BytesToRead;
            break;
        }
    }

    if (i == NUM_INPUT_SWAP)
    {
        status = STATUS_DATA_ERROR;
        TErr(("(OemUpdateInputReadLen) no match buffer."));
    }

    TExit(Func, ("=%x", status));
    return status;
}       //OemUpdateInputReadLen

ULONG INTERNAL
OemGetInputReadLen(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
    ULONG i, BytesToRead = 0;

    TEnter(Func, ("(InputBuffers=%p,Irp=%p)", InputBuffers, Irp));
	printk(KERN_INFO "[SIS] InputBuffers->InputBuffer[0].BytesToRead = %d\n",
		   InputBuffers->InputBuffer[0].BytesToRead);
	BytesToRead = InputBuffers->InputBuffer[0].BytesToRead;
#if 0
	for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (InputBuffers->InputBuffer[i].Irp == Irp)
        {
            BytesToRead = InputBuffers->InputBuffer[i].BytesToRead;
			printk(KERN_INFO "[SIS_DRIVER] BytesToRead = %d\n", BytesToRead);
            break;
        }
    }

    if (i == NUM_INPUT_SWAP)
    {
        TErr(("(OemGetInputReadLen) no match buffer."));
    }
    TExit(Func, ("=%d", BytesToRead));
#endif
    return BytesToRead;
}       //OemGetInputReadLen

ULONG INTERNAL
OemGetInputStamp(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
    ULONG i, Stamp = 0;

    TEnter(Func, ("(InputBuffers=%p,Irp=%p)", InputBuffers, Irp));

    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (InputBuffers->InputBuffer[i].Irp == Irp)
        {
            Stamp = InputBuffers->InputBuffer[i].Stamp;
            break;
        }
    }

    if (i == NUM_INPUT_SWAP)
    {
        TErr(("(OemGetInputStamp) no match buffer."));
    }

    TExit(Func, ("=%d", Stamp));
    return Stamp;
}       //OemGetInputStamp

VOID INTERNAL
OemInputMoveOffset(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp,
    __in     ULONG          Offset
    )
{
    ULONG i;

    TEnter(Func, ("(InputBuffers=%p)", InputBuffers));

    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (InputBuffers->InputBuffer[i].Irp == Irp)
        {
            break;
        }
    }

    if (i == NUM_INPUT_SWAP)
    {
        TErr(("(OemInputMoveOffset) no match buffer."));
        return;
    }

    InputBuffers->InputBuffer[i].BytesToRead -= Offset;
    if (InputBuffers->InputBuffer[i].BytesToRead > 0)
    {
        MEMMOVE(InputBuffers->InputBuffer[i].RawInput,
                InputBuffers->InputBuffer[i].RawInput + Offset,
                InputBuffers->InputBuffer[i].BytesToRead);
    }

    TExit(Func, ("!"));
    return;
}       //OemInputMoveOffset

//
// Resync buffer operate functions (for extern)
//

BOOLEAN INTERNAL
OemIsResyncEmpty(
    __in     PRESYNC_BUFFER ResyncBuffer
    )
{
    BOOLEAN rc = FALSE;

    TEnter(Func, ("(ResyncBuffer=%p)", ResyncBuffer));
    if (ResyncBuffer->BytesInBuff == 0)
    {
        rc = TRUE;
    }

    TExit(Func, ("=%x", rc));
    return rc;
}       //OemResyncCat

ULONG INTERNAL
OemGetResyncRest(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     PINPUT_BUFFERS InputBuffers
    )
{
    ULONG i, BytesToRead = 0;
    ULONG SyncStamp = ResyncBuffer->Stamp;
    ULONG BytesInBuff = ResyncBuffer->BytesInBuff;

    TEnter(Func, ("(ResyncBuffer=%p,InputBuffers=%p)", ResyncBuffer, InputBuffers));

    for (i = 0; i < NUM_INPUT_SWAP; i++)
    {
        if (SyncStamp == InputBuffers->InputBuffer[i].Stamp)
        {
            TInfo(("(OemGetResyncRest) SyncStamp %d match InputBuffers->Stamp.", SyncStamp));
        }

        if (InputBuffers->InputBuffer[i].Irp)
        {
            BytesInBuff += InputBuffers->InputBuffer[i].BytesToRead;
        }
    }

    if (BytesInBuff > SIZE_INPUT_SYNC_BUFF)
    {
        TErr(("(OemGetResyncRest) BytesInBuff %d > buffer size %d.", BytesInBuff, SIZE_INPUT_SYNC_BUFF));

        TExit(Func, ("=%d", BytesToRead));
        return BytesToRead;
    }

    BytesToRead = (SIZE_RAW_INPUT_BUFF) - (BytesInBuff % (SIZE_RAW_INPUT_BUFF));
    if ((BytesInBuff + BytesToRead) > SIZE_INPUT_SYNC_BUFF)
    {
        TErr(("(OemGetResyncRest) BytesInBuff + BytesToRead %d + %d > buffer size %d.", BytesInBuff, BytesToRead, SIZE_INPUT_SYNC_BUFF));
        BytesToRead = SIZE_INPUT_SYNC_BUFF - BytesInBuff;
    }

    TExit(Func, ("=%d", BytesToRead));
    return BytesToRead;
}       //OemGetResyncRest

VOID INTERNAL
OemResyncMoveOffset(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     ULONG          Offset
    )
{
    TEnter(Func, ("(ResyncBuffer=%p)", ResyncBuffer));

    ResyncBuffer->BytesInBuff -= Offset;
    if (ResyncBuffer->BytesInBuff > 0)
    {
        MEMMOVE(&ResyncBuffer->ResyncData[0],
                &ResyncBuffer->ResyncData[0] + Offset,
                ResyncBuffer->BytesInBuff);
        MEMSETZERO(&ResyncBuffer->ResyncData[0] + Offset, ResyncBuffer->BytesInBuff); // jiunhau
    }

    TExit(Func, ("!"));
    return;
}       //OemResyncMoveOffset

VOID INTERNAL
OemResyncCat(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     PUCHAR         CatBuffer,
    __in     ULONG          CatLength
    )
{
    TEnter(Func, ("(ResyncBuffer=%p)", ResyncBuffer));
    MEMMOVE((PUCHAR)&ResyncBuffer->ResyncData[0] + ResyncBuffer->BytesInBuff, CatBuffer, CatLength);
    ResyncBuffer->BytesInBuff += CatLength;

    TExit(Func, ("!"));
    return;
}       //OemResyncCat

VOID INTERNAL
OemResyncCatInput(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
    PUCHAR RawInput = OemGetInputBuff(InputBuffers, Irp);
    ULONG BytesToRead = OemGetInputReadLen(InputBuffers, Irp);

    TEnter(Func, ("(ResyncBuffer=%p)", ResyncBuffer));

#ifndef LINUX
    TAssert(ResyncBuffer->BytesInBuff <= sizeof(ResyncBuffer->ResyncData));
#endif
    if (ResyncBuffer->BytesInBuff > sizeof(ResyncBuffer->ResyncData)) //SIZE_INPUT_SYNC_BUFF
    {
        TErr(("(OemResyncCatInput) BytesInBuff %d > buffer size %d.", ResyncBuffer->BytesInBuff, sizeof(ResyncBuffer->ResyncData)));
        BytesToRead = sizeof(ResyncBuffer->ResyncData) - ResyncBuffer->BytesInBuff;
        //printk(KERN_INFO "[QQ] BytesToRead = %d | ResyncData = %d | BytesInBuff = %d\n", BytesToRead, sizeof(ResyncBuffer->ResyncData), ResyncBuffer->BytesInBuff);
    }
    OemResyncCat(ResyncBuffer, RawInput, BytesToRead);
    ResyncBuffer->Stamp = OemGetInputStamp(InputBuffers, Irp);
    OemResetInputBuff(InputBuffers, Irp);
    MTInfo(MSGFLTR_INPUTDATA, ("(OemResyncCatInput) ResyncBuffer->Stamp %d, InputBuffer %p.\n", ResyncBuffer->Stamp, RawInput));
    PrintResync(ResyncBuffer);
    TExit(Func, ("!"));
    return;
}       //OemResyncCatInput

VOID INTERNAL
PrintInput(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    )
{
//    PUCHAR RawInput = OemGetInputBuff(InputBuffers, Irp);
//    ULONG BytesToRead = OemGetInputReadLen(InputBuffers, Irp);

#if 1
    MTInfo(MSGFLTR_INPUTDATA, ("InputBuffer Irp: %p.", Irp));
    PrintMatrix(MSGFLTR_INPUTDATA, RawInput, BytesToRead, 16); //jiunhau
	PrintMatrix(MSGFLTR_INPUTDATA, OemGetInputBuff(InputBuffers, Irp),
				OemGetInputReadLen(InputBuffers, Irp), 16);
#endif
#if 0	// jiunhau
	int i, Align = 4, length = OemGetInputReadLen(InputBuffers, Irp);
	PUCHAR buf = OemGetInputBuff(InputBuffers, Irp);
	for (i = 0; i < length; i++)
	{
		printk(KERN_INFO "buf[%d] = %2x ", i, buf[i]);
		if (!((i + 1) % Align))
		{
			printk(KERN_INFO "\n");
		}
	}
	printk(KERN_INFO "\n");
#endif
    return;
}


VOID INTERNAL
PrintResync(
    __in     PRESYNC_BUFFER ResyncBuffer
    )
{
    MTInfo(MSGFLTR_INPUTDATA, ("ResyncBuffer Stamp: %X.", ResyncBuffer->Stamp));
    PrintMatrix(MSGFLTR_INPUTDATA, ResyncBuffer->ResyncData, ResyncBuffer->BytesInBuff, 16);

    return;
}
