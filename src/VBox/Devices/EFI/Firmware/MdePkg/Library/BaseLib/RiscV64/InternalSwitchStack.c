/** @file
  Switch stack function for RISC-V

  Copyright (c) 2020, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "BaseLibInternals.h"

/**
  Transfers control to a function starting with a new stack.

  This internal worker function transfers control to the function
  specified by EntryPoint using the new stack specified by NewStack,
  and passes in the parameters specified by Context1 and Context2.
  Context1 and Context2 are optional and may be NULL.
  The function EntryPoint must never return.

  @param Context1     The first parameter to pass in.
  @param Context2     The second Parameter to pass in
  @param EntryPoint   The pointer to the function to enter.
  @param NewStack     The new Location of the stack

**/
VOID
EFIAPI
InternalSwitchStackAsm (
  IN      VOID                      *Context1    OPTIONAL,
  IN      VOID                      *Context2    OPTIONAL,
  IN      SWITCH_STACK_ENTRY_POINT  EntryPoint,
  IN      VOID                      *NewStack
  );

/**
  Transfers control to a function starting with a new stack.

  Transfers control to the function specified by EntryPoint using the
  new stack specified by NewStack and passing in the parameters specified
  by Context1 and Context2.  Context1 and Context2 are optional and may
  be NULL.  The function EntryPoint must never return.
  Marker will be ignored on IA-32, x64, and EBC.
  IPF CPUs expect one additional parameter of type VOID * that specifies
  the new backing store pointer.

  If EntryPoint is NULL, then ASSERT().
  If NewStack is NULL, then ASSERT().

  @param  EntryPoint  A pointer to function to call with the new stack.
  @param  Context1    A pointer to the context to pass into the EntryPoint
                      function.
  @param  Context2    A pointer to the context to pass into the EntryPoint
                      function.
  @param  NewStack    A pointer to the new stack to use for the EntryPoint
                      function.
  @param  Marker      VA_LIST marker for the variable argument list.

**/
VOID
EFIAPI
InternalSwitchStack (
  IN      SWITCH_STACK_ENTRY_POINT  EntryPoint,
  IN      VOID                      *Context1    OPTIONAL,
  IN      VOID                      *Context2    OPTIONAL,
  IN      VOID                      *NewStack,
  IN      VA_LIST                   Marker
  )
{
  InternalSwitchStackAsm (Context1, Context2, EntryPoint, (VOID *)((UINTN)NewStack - sizeof (VOID *)));
  ASSERT (FALSE);
}
