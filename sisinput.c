/*++
    Copyright (c) Silicon Integrated Systems Corporation

    Module Name:
        oemtp.c

    Abstract:
        Contains OEM process input data functions.

    Environment:
        Kernel mode

--*/

#include "sisinput.h"
//#include "common.h"
#define MODULE_ID                       MODULE_SISINPUT
#define MSGMDL_ID                       MSGMDL_SISINPUT

//
// Key definitions
//
#define DRIVERINFO printk(KERN_INFO "[sisinput] %s : %d\n", __func__, __LINE__)

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

//sisvalid.c

BOOLEAN INTERNAL
OemIsFrameValid(
	__in PUCHAR pbStart,
	__in PUCHAR pbEnd,
    __out PRAW_PARSER RawParser,
    __out PCOMMAND_INFO CommandInfo
    );

BOOLEAN INTERNAL
OemIsResyncDataValid(
    __in PRESYNC_BUFFER    ResyncBuffer,
    __out PRAW_PARSER      RawParser,
    __out PCOMMAND_INFO    CommandInfo
    );

ULONG INTERNAL
OemGetParserReadStatus(
    __in     PRAW_PARSER RawParser
    );

ULONG INTERNAL
OemGetProcessedRawDataLen(
    __in     PRAW_PARSER RawParser
    );


//sistrans.c

NTSTATUS INTERNAL
OemNormalizeInputData(
    __in     PCOMMAND_INFO     CommandInfo,
    __inout  POEM_INPUT_REPORT InData
    );

// sisinbuf.c

PUCHAR INTERNAL
OemGetInputBuff(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    );

ULONG INTERNAL
OemGetInputReadLen(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    );

VOID INTERNAL
OemInputMoveOffset(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp,
    __in     ULONG          Offset
    );

BOOLEAN INTERNAL
OemIsResyncEmpty(
    __in     PRESYNC_BUFFER ResyncBuffer
    );

VOID INTERNAL
OemResyncMoveOffset(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     ULONG          Offset
    );

VOID INTERNAL
OemResyncCatInput(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    );

VOID INTERNAL
PrintInput(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp
    );

VOID INTERNAL
PrintResync(
    __in     PRESYNC_BUFFER ResyncBuffer
    );


//
// Local function prototype.
//


//
// Global variable definitions
//
static int run_time = 0;

//
// Precess input data functions
//

/*++
    @doc    INTERNAL

    @func   NTSTATUS | ProcessResyncBuffer |
            Process input data from the resync buffer.
            Note that this function must be called at IRQL==DISPATCH_LEVEL

    @parm   INOUT OUT PRESYNC_BUFFER | ResyncBuffer | Points to the resync buffer.
    @parm   INOUT PRAW_PARSER | RawParser | Points to the raw data parser status.
    @parm   IN PHID_INPUT_REPORT | HidReport | Points to the output report.
    @parm   IN BOOLEAN | OnlyValid | for only valid frame not process data.

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_MORE_PROCESSING_REQUIRED
--*/
NTSTATUS INTERNAL
ProcessResyncBuffer(
    __inout PRESYNC_BUFFER      ResyncBuffer,
    __inout PRAW_PARSER         RawParser,
    __out   PHID_INPUT_REPORT   HidReport,
    __in    BOOLEAN             OnlyValid
    )
{
    NTSTATUS status = STATUS_DATA_ERROR;
    COMMAND_INFO CommandInfo = {0};
    BOOLEAN ReadMoreBytes = FALSE;
    ULONG Offset = 0;

    TEnter(Func, ("(ResyncBuffer=%p,RawParser=%p,HidReport=%p)",
                  ResyncBuffer, RawParser, HidReport));

#ifndef LINUX
    TAssert(KeGetCurrentIrql() == DISPATCH_LEVEL);
#endif

    while (!OemIsResyncEmpty(ResyncBuffer) && !ReadMoreBytes)
    {
        if (OemIsResyncDataValid(ResyncBuffer, RawParser, &CommandInfo))
        {
            if (OnlyValid)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
            	//DRIVERINFO;
                status = OemNormalizeInputData(&CommandInfo,
                                               &HidReport->Report);
            }

            Offset = OemGetProcessedRawDataLen (RawParser);
            OemResyncMoveOffset(ResyncBuffer, Offset);
            //DRIVERINFO;
            MTInfo(MSGFLTR_INPUTDATA, ("(ProcessResyncBuffer)."));
            PrintResync(ResyncBuffer);

            if (NT_SUCCESS(status))
            {
                break;
            }
        }
        else
        {
        	//DRIVERINFO;
            ReadMoreBytes = (BOOLEAN)OemGetParserReadStatus(RawParser);
        }
    }

    TExit(Func, ("=%x", status));
    return status;
}//ProcessResyncBuffer

/*++
    @doc    INTERNAL

    @func   NTSTATUS | ProcessInputData |
            OEM specific code to process input data.

    @parm   OUT PINPUT_BUFFERS | InputBuffers | Points to the raw input swap buffer.
    @parm   OUT PRESYNC_BUFFER | ResyncBuffer | Points to the resync buffer.
    @parm   OUT PRAW_PARSER | RawParser | Points to the raw data parser status.
    @parm   IN PIRP | Irp | Points to an I/O request packet.
    @parm   OUT PHID_INPUT_REPORT | HidReport | Points to hid report packet.
    @parm   IN BOOLEAN | OnlyValid | for only valid frame not process data.

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_MORE_PROCESSING_REQUIRED
--*/
NTSTATUS INTERNAL
ProcessInputData(
    __in    PINPUT_BUFFERS      InputBuffers,
    __out   PRESYNC_BUFFER      ResyncBuffer,
    __out   PRAW_PARSER         RawParser,
    __in    PIRP                Irp,
    __out   PHID_INPUT_REPORT   HidReport,
    __in    BOOLEAN             OnlyValid
    )
{
    NTSTATUS status;
    COMMAND_INFO CommandInfo = {0};
    PUCHAR RawInput;
    int i = 0;
    ULONG Offset;
    ULONG BytesToRead = OemGetInputReadLen(InputBuffers, Irp);
	TEnter(Func,
           ("(InputBuffers=%p,ResyncBuffer=%p,RawParser=%p,Irp=%p,HidReport=%p)",
            InputBuffers, ResyncBuffer, RawParser, Irp, HidReport));

    MTInfo(MSGFLTR_INPUTDATA, ("ProcessInputData (1) snyc data count %d, information value %d.", ResyncBuffer->BytesInBuff, BytesToRead));
    MTInfo(MSGFLTR_INPUTDATA, ("(ProcessInputData) InputBuffer Irp %p.", Irp));
    PrintInput (InputBuffers, Irp);
    RawInput = OemGetInputBuff (InputBuffers, Irp);
#if 0
    for(i = 0; i < BytesToRead; i++)
    {
		printk(KERN_INFO "%x  ", RawInput[i]);
    }
    printk(KERN_INFO "\n");
#endif
    if (run_time == 0)
    {
    	ResyncBuffer->BytesInBuff = 0;
    	MEMSETZERO(&ResyncBuffer->ResyncData[0], SIZE_INPUT_SYNC_BUFF);
    	run_time++;
    }
    // jiunhau

	if (OemIsResyncEmpty(ResyncBuffer) &&
        OemIsFrameValid(RawInput, RawInput + BytesToRead, RawParser, &CommandInfo))
    {
        if (OnlyValid)
        {
            status = STATUS_SUCCESS;
			//DRIVERINFO;
        }
        else
        {
            status = OemNormalizeInputData(&CommandInfo, &HidReport->Report);
			//DRIVERINFO;
        }

        // move input buffer remainder to resync buffer
        Offset = OemGetProcessedRawDataLen (RawParser);
	//	DRIVERINFO;
		OemInputMoveOffset(InputBuffers, Irp, Offset);
		//DRIVERINFO;
		OemResyncCatInput(ResyncBuffer, InputBuffers, Irp);
		//DRIVERINFO;
    }
    else
    {
        //
        // Either resync buffer already has something in it or packet is
        // partial or invalid, so append data to resync buffer and process
        // it again.
        //
        MTInfo(MSGFLTR_INPUTDATA, ("ProcessInputData (2) snyc data count %d, information value %d.", ResyncBuffer->BytesInBuff, BytesToRead));
        if (ResyncBuffer->BytesInBuff > 0x100)
        {
            MTWarn(MSGFLTR_INPUTDATA, ("snyc data count %d exceed 0x100, information value %d.", ResyncBuffer->BytesInBuff, BytesToRead));
        }
        if ((ResyncBuffer->BytesInBuff + BytesToRead) > 0x180)
        {
            MTErr(MSGFLTR_INPUTDATA, ("snyc data count %d exceed buffer length, information value %d.", ResyncBuffer->BytesInBuff, BytesToRead));
        }

        OemResyncCatInput(ResyncBuffer, InputBuffers, Irp);
		//DRIVERINFO; // QQ
        MTInfo(MSGFLTR_INPUTDATA, ("ProcessInputData (3) snyc data count %d, information value %d.", ResyncBuffer->BytesInBuff, BytesToRead));

        status = ProcessResyncBuffer(ResyncBuffer, RawParser, HidReport, OnlyValid);
		//DRIVERINFO; // QQ
		if (ResyncBuffer->BytesInBuff > 0x180)
        {
            MTErr(MSGFLTR_INPUTDATA, ("snyc data count %d exceed buffer length.", ResyncBuffer->BytesInBuff));
        }
        MTInfo(MSGFLTR_INPUTDATA, ("ProcessInputData (4) snyc data count %d, information value %d.", ResyncBuffer->BytesInBuff, BytesToRead));
    }

    TExit(Func, ("=%x", status));
    return status;
}       //ProcessInputData
