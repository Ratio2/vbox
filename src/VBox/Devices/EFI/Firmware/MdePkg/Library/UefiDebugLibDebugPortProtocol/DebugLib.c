/** @file
  UEFI Debug Library that sends messages to EFI_DEBUGPORT_PROTOCOL.Write.

  Copyright (c) 2015 - 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugPrintErrorLevelLib.h>

#include <Protocol/DebugPort.h>

//
// Define the maximum debug and assert message length that this library supports
//
#define MAX_DEBUG_MESSAGE_LENGTH  0x100

//
// Define the timeout for EFI_DEBUGPORT_PROTOCOL.Write
//
#define WRITE_TIMEOUT  1000

EFI_DEBUGPORT_PROTOCOL  *mDebugPort = NULL;

//
// VA_LIST can not initialize to NULL for all compiler, so we use this to
// indicate a null VA_LIST
//
VA_LIST  mVaListNull;

extern BOOLEAN            mPostEBS;
extern EFI_BOOT_SERVICES  *mDebugBS;

/**
  Send message to DebugPort Protocol.

  If mDebugPort is NULL, i.e. EFI_DEBUGPORT_PROTOCOL is not located,
  EFI_DEBUGPORT_PROTOCOL is located first.
  Then, Buffer is sent via EFI_DEBUGPORT_PROTOCOL.Write.

  @param  Buffer         The message to be sent.
  @param  BufferLength   The byte length of Buffer.
**/
VOID
UefiDebugLibDebugPortProtocolWrite (
  IN  CONST CHAR8  *Buffer,
  IN        UINTN  BufferLength
  )
{
  UINTN       Length;
  EFI_STATUS  Status;

  if (!mPostEBS) {
    //
    // If mDebugPort is NULL, initialize first.
    //
    if (mDebugPort == NULL) {
      Status = mDebugBS->LocateProtocol (&gEfiDebugPortProtocolGuid, NULL, (VOID **)&mDebugPort);
      if (EFI_ERROR (Status)) {
        return;
      }

      mDebugPort->Reset (mDebugPort);
    }

    //
    // EFI_DEBUGPORT_PROTOCOL.Write is called until all message is sent.
    //
    while (BufferLength > 0) {
      Length = BufferLength;

      Status = mDebugPort->Write (mDebugPort, WRITE_TIMEOUT, &Length, (VOID *)Buffer);
      if (EFI_ERROR (Status) || (BufferLength < Length)) {
        break;
      }

      Buffer       += Length;
      BufferLength -= Length;
    }
  }
}

/**
  Prints a debug message to the debug output device if the specified error level is enabled.

  If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
  GetDebugPrintErrorLevel (), then print the message specified by Format and the
  associated variable argument list to the debug output device.

  If Format is NULL, then ASSERT().

  @param  ErrorLevel  The error level of the debug message.
  @param  Format      Format string for the debug message to print.
  @param  ...         A variable argument list whose contents are accessed
                      based on the format string specified by Format.

**/
VOID
EFIAPI
DebugPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  ...
  )
{
  VA_LIST  Marker;

  VA_START (Marker, Format);
  DebugVPrint (ErrorLevel, Format, Marker);
  VA_END (Marker);
}

/**
  Prints a debug message to the debug output device if the specified
  error level is enabled base on Null-terminated format string and a
  VA_LIST argument list or a BASE_LIST argument list.

  If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
  GetDebugPrintErrorLevel (), then print the message specified by Format and
  the associated variable argument list to the debug output device.

  If Format is NULL, then ASSERT().

  @param  ErrorLevel      The error level of the debug message.
  @param  Format          Format string for the debug message to print.
  @param  VaListMarker    VA_LIST marker for the variable argument list.
  @param  BaseListMarker  BASE_LIST marker for the variable argument list.

**/
VOID
DebugPrintMarker (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  IN  VA_LIST      VaListMarker,
  IN  BASE_LIST    BaseListMarker
  )
{
  CHAR8  Buffer[MAX_DEBUG_MESSAGE_LENGTH];

  if (!mPostEBS) {
    //
    // If Format is NULL, then ASSERT().
    //
    ASSERT (Format != NULL);

    //
    // Check driver debug mask value and global mask
    //
    if ((ErrorLevel & GetDebugPrintErrorLevel ()) == 0) {
      return;
    }

    //
    // Convert the DEBUG() message to an ASCII String
    //
    if (BaseListMarker == NULL) {
      AsciiVSPrint (Buffer, sizeof (Buffer), Format, VaListMarker);
    } else {
      AsciiBSPrint (Buffer, sizeof (Buffer), Format, BaseListMarker);
    }

    //
    // Send the print string to EFI_DEBUGPORT_PROTOCOL.Write.
    //
    UefiDebugLibDebugPortProtocolWrite (Buffer, AsciiStrLen (Buffer));
  }
}

/**
  Prints a debug message to the debug output device if the specified
  error level is enabled.

  If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
  GetDebugPrintErrorLevel (), then print the message specified by Format and
  the associated variable argument list to the debug output device.

  If Format is NULL, then ASSERT().

  @param  ErrorLevel    The error level of the debug message.
  @param  Format        Format string for the debug message to print.
  @param  VaListMarker  VA_LIST marker for the variable argument list.

**/
VOID
EFIAPI
DebugVPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  IN  VA_LIST      VaListMarker
  )
{
  DebugPrintMarker (ErrorLevel, Format, VaListMarker, NULL);
}

/**
  Prints a debug message to the debug output device if the specified
  error level is enabled.
  This function use BASE_LIST which would provide a more compatible
  service than VA_LIST.

  If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
  GetDebugPrintErrorLevel (), then print the message specified by Format and
  the associated variable argument list to the debug output device.

  If Format is NULL, then ASSERT().

  @param  ErrorLevel      The error level of the debug message.
  @param  Format          Format string for the debug message to print.
  @param  BaseListMarker  BASE_LIST marker for the variable argument list.

**/
VOID
EFIAPI
DebugBPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  IN  BASE_LIST    BaseListMarker
  )
{
  DebugPrintMarker (ErrorLevel, Format, mVaListNull, BaseListMarker);
}

/**
  Prints an assert message containing a filename, line number, and description.
  This may be followed by a breakpoint or a dead loop.

  Print a message of the form "ASSERT <FileName>(<LineNumber>): <Description>\n"
  to the debug output device.  If DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED bit of
  PcdDebugPropertyMask is set then CpuBreakpoint() is called. Otherwise, if
  DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED bit of PcdDebugPropertyMask is set then
  CpuDeadLoop() is called.  If neither of these bits are set, then this function
  returns immediately after the message is printed to the debug output device.
  DebugAssert() must actively prevent recursion.  If DebugAssert() is called while
  processing another DebugAssert(), then DebugAssert() must return immediately.

  If FileName is NULL, then a <FileName> string of "(NULL) Filename" is printed.
  If Description is NULL, then a <Description> string of "(NULL) Description" is printed.

  @param  FileName     The pointer to the name of the source file that generated
                       the assert condition.
  @param  LineNumber   The line number in the source file that generated the
                       assert condition
  @param  Description  The pointer to the description of the assert condition.

**/
VOID
EFIAPI
DebugAssert (
  IN CONST CHAR8  *FileName,
  IN UINTN        LineNumber,
  IN CONST CHAR8  *Description
  )
{
  CHAR8  Buffer[MAX_DEBUG_MESSAGE_LENGTH];

  if (!mPostEBS) {
    //
    // Generate the ASSERT() message in ASCII format
    //
    AsciiSPrint (
      Buffer,
      sizeof (Buffer),
      "ASSERT [%a] %a(%d): %a\n",
      gEfiCallerBaseName,
      FileName,
      LineNumber,
      Description
      );

    //
    // Send the print string to EFI_DEBUGPORT_PROTOCOL.Write.
    //
    UefiDebugLibDebugPortProtocolWrite (Buffer, AsciiStrLen (Buffer));

    //
    // Generate a Breakpoint, DeadLoop, or NOP based on PCD settings
    //
    if ((PcdGet8 (PcdDebugPropertyMask) & DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED) != 0) {
      CpuBreakpoint ();
    } else if ((PcdGet8 (PcdDebugPropertyMask) & DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED) != 0) {
      CpuDeadLoop ();
    }
  }
}

/**
  Fills a target buffer with PcdDebugClearMemoryValue, and returns the target buffer.

  This function fills Length bytes of Buffer with the value specified by
  PcdDebugClearMemoryValue, and returns Buffer.

  If Buffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param   Buffer  The pointer to the target buffer to be filled with PcdDebugClearMemoryValue.
  @param   Length  The number of bytes in Buffer to fill with zeros PcdDebugClearMemoryValue.

  @return  Buffer  The pointer to the target buffer filled with PcdDebugClearMemoryValue.

**/
VOID *
EFIAPI
DebugClearMemory (
  OUT VOID  *Buffer,
  IN UINTN  Length
  )
{
  //
  // If Buffer is NULL, then ASSERT().
  //
  ASSERT (Buffer != NULL);

  //
  // SetMem() checks for the the ASSERT() condition on Length and returns Buffer
  //
  return SetMem (Buffer, Length, PcdGet8 (PcdDebugClearMemoryValue));
}

/**
  Returns TRUE if ASSERT() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugAssertEnabled (
  VOID
  )
{
  return (BOOLEAN)((PcdGet8 (PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED) != 0);
}

/**
  Returns TRUE if DEBUG() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugPrintEnabled (
  VOID
  )
{
  return (BOOLEAN)((PcdGet8 (PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_PRINT_ENABLED) != 0);
}

/**
  Returns TRUE if DEBUG_CODE() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugCodeEnabled (
  VOID
  )
{
  return (BOOLEAN)((PcdGet8 (PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_CODE_ENABLED) != 0);
}

/**
  Returns TRUE if DEBUG_CLEAR_MEMORY() macro is enabled.

  This function returns TRUE if the DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugClearMemoryEnabled (
  VOID
  )
{
  return (BOOLEAN)((PcdGet8 (PcdDebugPropertyMask) & DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED) != 0);
}

/**
  Returns TRUE if any one of the bit is set both in ErrorLevel and PcdFixedDebugPrintErrorLevel.

  This function compares the bit mask of ErrorLevel and PcdFixedDebugPrintErrorLevel.

  @retval  TRUE    Current ErrorLevel is supported.
  @retval  FALSE   Current ErrorLevel is not supported.

**/
BOOLEAN
EFIAPI
DebugPrintLevelEnabled (
  IN  CONST UINTN  ErrorLevel
  )
{
  return (BOOLEAN)((ErrorLevel & PcdGet32 (PcdFixedDebugPrintErrorLevel)) != 0);
}
