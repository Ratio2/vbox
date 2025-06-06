/** @file
  The AhciPei driver is used to manage ATA hard disk device working under AHCI
  mode at PEI phase.

  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "AhciPei.h"

/**
  Traverse the attached ATA devices list to find out the device with given Port
  and PortMultiplierPort.

  @param[in] Private               A pointer to the PEI_AHCI_CONTROLLER_PRIVATE_DATA
                                   instance.
  @param[in] Port                  The port number of the ATA device.
  @param[in] PortMultiplierPort    The port multiplier port number of the ATA device.

  @retval    The pointer to the PEI_AHCI_ATA_DEVICE_DATA structure of the device
             info to access.

**/
PEI_AHCI_ATA_DEVICE_DATA *
SearchDeviceByPort (
  IN PEI_AHCI_CONTROLLER_PRIVATE_DATA  *Private,
  IN UINT16                            Port,
  IN UINT16                            PortMultiplierPort
  )
{
  PEI_AHCI_ATA_DEVICE_DATA  *DeviceData;
  LIST_ENTRY                *Node;

  Node = GetFirstNode (&Private->DeviceList);
  while (!IsNull (&Private->DeviceList, Node)) {
    DeviceData = AHCI_PEI_ATA_DEVICE_INFO_FROM_THIS (Node);

    if ((DeviceData->Port == Port) &&
        (DeviceData->PortMultiplier == PortMultiplierPort))
    {
      return DeviceData;
    }

    Node = GetNextNode (&Private->DeviceList, Node);
  }

  return NULL;
}

/**
  Sends an ATA command to an ATA device that is attached to the ATA controller.

  @param[in]     Private               Pointer to the PEI_AHCI_CONTROLLER_PRIVATE_DATA.
  @param[in]     Port                  The port number of the ATA device.
  @param[in]     PortMultiplierPort    The port multiplier port number of the ATA
                                       device.
  @param[in]     FisIndex              The index of the FIS.
  @param[in,out] Packet                A pointer to the ATA command to send to
                                       the ATA device specified by Port and
                                       PortMultiplierPort.

  @retval EFI_SUCCESS              The ATA command was sent by the host. For
                                   bi-directional commands, InTransferLength bytes
                                   were transferred from InDataBuffer. For write
                                   and bi-directional commands, OutTransferLength
                                   bytes were transferred by OutDataBuffer.
  @retval EFI_BAD_BUFFER_SIZE      The ATA command was not executed. The number
                                   of bytes that could be transferred is returned
                                   in InTransferLength. For write and bi-directional
                                   commands, OutTransferLength bytes were transferred
                                   by OutDataBuffer.
  @retval EFI_NOT_READY            The ATA command could not be sent because there
                                   are too many ATA commands already queued. The
                                   caller may retry again later.
  @retval EFI_DEVICE_ERROR         A device error occurred while attempting to
                                   send the ATA command.
  @retval EFI_INVALID_PARAMETER    Port, PortMultiplierPort, or the contents of
                                   Acb are invalid. The ATA command was not sent,
                                   so no additional status information is available.

**/
EFI_STATUS
AhciPassThruExecute (
  IN     PEI_AHCI_CONTROLLER_PRIVATE_DATA  *Private,
  IN     UINT16                            Port,
  IN     UINT16                            PortMultiplierPort,
  IN     UINT8                             FisIndex,
  IN OUT EFI_ATA_PASS_THRU_COMMAND_PACKET  *Packet
  )
{
  EFI_STATUS  Status;

  if (PortMultiplierPort == 0xFFFF) {
    //
    // If there is no port multiplier, PortMultiplierPort will be 0xFFFF
    // according to UEFI spec. Here, we convert its value to 0 to follow
    // AHCI spec.
    //
    PortMultiplierPort = 0;
  }

  switch (Packet->Protocol) {
    case EFI_ATA_PASS_THRU_PROTOCOL_ATA_NON_DATA:
      Status = AhciNonDataTransfer (
                 Private,
                 (UINT8)Port,
                 (UINT8)PortMultiplierPort,
                 FisIndex,
                 Packet->Acb,
                 Packet->Asb,
                 Packet->Timeout
                 );
      break;
    case EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_IN:
      Status = AhciPioTransfer (
                 Private,
                 (UINT8)Port,
                 (UINT8)PortMultiplierPort,
                 FisIndex,
                 TRUE,
                 Packet->Acb,
                 Packet->Asb,
                 Packet->InDataBuffer,
                 Packet->InTransferLength,
                 Packet->Timeout
                 );
      break;
    case EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_OUT:
      Status = AhciPioTransfer (
                 Private,
                 (UINT8)Port,
                 (UINT8)PortMultiplierPort,
                 FisIndex,
                 FALSE,
                 Packet->Acb,
                 Packet->Asb,
                 Packet->OutDataBuffer,
                 Packet->OutTransferLength,
                 Packet->Timeout
                 );
      break;
    default:
      return EFI_UNSUPPORTED;
  }

  return Status;
}

/**
  Sends an ATA command to an ATA device that is attached to the ATA controller.

  @param[in]     This                  The PPI instance pointer.
  @param[in]     Port                  The port number of the ATA device to send
                                       the command.
  @param[in]     PortMultiplierPort    The port multiplier port number of the ATA
                                       device to send the command.
                                       If there is no port multiplier, then specify
                                       0xFFFF.
  @param[in,out] Packet                A pointer to the ATA command to send to
                                       the ATA device specified by Port and
                                       PortMultiplierPort.

  @retval EFI_SUCCESS              The ATA command was sent by the host. For
                                   bi-directional commands, InTransferLength bytes
                                   were transferred from InDataBuffer. For write
                                   and bi-directional commands, OutTransferLength
                                   bytes were transferred by OutDataBuffer.
  @retval EFI_NOT_FOUND            The specified ATA device is not found.
  @retval EFI_INVALID_PARAMETER    The contents of Acb are invalid. The ATA command
                                   was not sent, so no additional status information
                                   is available.
  @retval EFI_BAD_BUFFER_SIZE      The ATA command was not executed. The number
                                   of bytes that could be transferred is returned
                                   in InTransferLength. For write and bi-directional
                                   commands, OutTransferLength bytes were transferred
                                   by OutDataBuffer.
  @retval EFI_NOT_READY            The ATA command could not be sent because there
                                   are too many ATA commands already queued. The
                                   caller may retry again later.
  @retval EFI_DEVICE_ERROR         A device error occurred while attempting to
                                   send the ATA command.

**/
EFI_STATUS
EFIAPI
AhciAtaPassThruPassThru (
  IN     EDKII_PEI_ATA_PASS_THRU_PPI       *This,
  IN     UINT16                            Port,
  IN     UINT16                            PortMultiplierPort,
  IN OUT EFI_ATA_PASS_THRU_COMMAND_PACKET  *Packet
  )
{
  UINT32                            IoAlign;
  PEI_AHCI_CONTROLLER_PRIVATE_DATA  *Private;
  PEI_AHCI_ATA_DEVICE_DATA          *DeviceData;
  UINT32                            MaxSectorCount;
  UINT32                            BlockSize;

  if ((This == NULL) || (Packet == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  IoAlign = This->Mode->IoAlign;
  if ((IoAlign > 1) && !ADDRESS_IS_ALIGNED (Packet->InDataBuffer, IoAlign)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((IoAlign > 1) && !ADDRESS_IS_ALIGNED (Packet->OutDataBuffer, IoAlign)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((IoAlign > 1) && !ADDRESS_IS_ALIGNED (Packet->Asb, IoAlign)) {
    return EFI_INVALID_PARAMETER;
  }

  Private    = GET_AHCI_PEIM_HC_PRIVATE_DATA_FROM_THIS_PASS_THRU (This);
  DeviceData = SearchDeviceByPort (Private, Port, PortMultiplierPort);
  if (DeviceData == NULL) {
    return EFI_NOT_FOUND;
  }

  MaxSectorCount = mMaxTransferBlockNumber[DeviceData->Lba48Bit];
  BlockSize      = DeviceData->Media.BlockSize;

  //
  // Convert the transfer length from sector count to byte.
  //
  if (((Packet->Length & EFI_ATA_PASS_THRU_LENGTH_BYTES) == 0) &&
      (Packet->InTransferLength != 0))
  {
    Packet->InTransferLength = Packet->InTransferLength * BlockSize;
  }

  //
  // Convert the transfer length from sector count to byte.
  //
  if (((Packet->Length & EFI_ATA_PASS_THRU_LENGTH_BYTES) == 0) &&
      (Packet->OutTransferLength != 0))
  {
    Packet->OutTransferLength = Packet->OutTransferLength * BlockSize;
  }

  //
  // If the data buffer described by InDataBuffer/OutDataBuffer and
  // InTransferLength/OutTransferLength is too big to be transferred in a single
  // command, then no data is transferred and EFI_BAD_BUFFER_SIZE is returned.
  //
  if (((Packet->InTransferLength != 0) && (Packet->InTransferLength > MaxSectorCount * BlockSize)) ||
      ((Packet->OutTransferLength != 0) && (Packet->OutTransferLength > MaxSectorCount * BlockSize)))
  {
    return EFI_BAD_BUFFER_SIZE;
  }

  return AhciPassThruExecute (
           Private,
           DeviceData->Port,
           DeviceData->PortMultiplier,
           DeviceData->FisIndex,
           Packet
           );
}

/**
  Used to retrieve the list of legal port numbers for ATA devices on an ATA controller.
  These can either be the list of ports where ATA devices are actually present or the
  list of legal port numbers for the ATA controller. Regardless, the caller of this
  function must probe the port number returned to see if an ATA device is actually
  present at that location on the ATA controller.

  The GetNextPort() function retrieves the port number on an ATA controller. If on
  input Port is 0xFFFF, then the port number of the first port on the ATA controller
  is returned in Port and EFI_SUCCESS is returned.

  If Port is a port number that was returned on a previous call to GetNextPort(),
  then the port number of the next port on the ATA controller is returned in Port,
  and EFI_SUCCESS is returned. If Port is not 0xFFFF and Port was not returned on
  a previous call to GetNextPort(), then EFI_INVALID_PARAMETER is returned.

  If Port is the port number of the last port on the ATA controller, then EFI_NOT_FOUND
  is returned.

  @param[in]     This    The PPI instance pointer.
  @param[in,out] Port    On input, a pointer to the port number on the ATA controller.
                         On output, a pointer to the next port number on the ATA
                         controller. An input value of 0xFFFF retrieves the first
                         port number on the ATA controller.

  @retval EFI_SUCCESS              The next port number on the ATA controller was
                                   returned in Port.
  @retval EFI_NOT_FOUND            There are no more ports on this ATA controller.
  @retval EFI_INVALID_PARAMETER    Port is not 0xFFFF and Port was not returned
                                   on a previous call to GetNextPort().

**/
EFI_STATUS
EFIAPI
AhciAtaPassThruGetNextPort (
  IN     EDKII_PEI_ATA_PASS_THRU_PPI  *This,
  IN OUT UINT16                       *Port
  )
{
  PEI_AHCI_CONTROLLER_PRIVATE_DATA  *Private;
  PEI_AHCI_ATA_DEVICE_DATA          *DeviceData;
  LIST_ENTRY                        *Node;

  if ((This == NULL) || (Port == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = GET_AHCI_PEIM_HC_PRIVATE_DATA_FROM_THIS_PASS_THRU (This);

  if (*Port == 0xFFFF) {
    //
    // If the Port is all 0xFF's, start to traverse the device list from the
    // beginning.
    //
    Node = GetFirstNode (&Private->DeviceList);
    if (!IsNull (&Private->DeviceList, Node)) {
      DeviceData = AHCI_PEI_ATA_DEVICE_INFO_FROM_THIS (Node);

      *Port = DeviceData->Port;
      goto Exit;
    }

    return EFI_NOT_FOUND;
  } else if (*Port == Private->PreviousPort) {
    Node = GetFirstNode (&Private->DeviceList);

    while (!IsNull (&Private->DeviceList, Node)) {
      DeviceData = AHCI_PEI_ATA_DEVICE_INFO_FROM_THIS (Node);

      if (DeviceData->Port > *Port) {
        *Port = DeviceData->Port;
        goto Exit;
      }

      Node = GetNextNode (&Private->DeviceList, Node);
    }

    return EFI_NOT_FOUND;
  } else {
    //
    // Port is not equal to all 0xFF's and not equal to previous return value.
    //
    return EFI_INVALID_PARAMETER;
  }

Exit:
  //
  // Update the PreviousPort.
  //
  Private->PreviousPort = *Port;

  return EFI_SUCCESS;
}

/**
  Used to retrieve the list of legal port multiplier port numbers for ATA devices
  on a port of an ATA controller. These can either be the list of port multiplier
  ports where ATA devices are actually present on port or the list of legal port
  multiplier ports on that port. Regardless, the caller of this function must probe
  the port number and port multiplier port number returned to see if an ATA device
  is actually present.

  The GetNextDevice() function retrieves the port multiplier port number of an ATA
  device present on a port of an ATA controller.

  If PortMultiplierPort points to a port multiplier port number value that was
  returned on a previous call to GetNextDevice(), then the port multiplier port
  number of the next ATA device on the port of the ATA controller is returned in
  PortMultiplierPort, and EFI_SUCCESS is returned.

  If PortMultiplierPort points to 0xFFFF, then the port multiplier port number
  of the first ATA device on port of the ATA controller is returned in PortMultiplierPort
  and EFI_SUCCESS is returned.

  If PortMultiplierPort is not 0xFFFF and the value pointed to by PortMultiplierPort
  was not returned on a previous call to GetNextDevice(), then EFI_INVALID_PARAMETER
  is returned.

  If PortMultiplierPort is the port multiplier port number of the last ATA device
  on the port of the ATA controller, then EFI_NOT_FOUND is returned.

  When port multiplier is not connected to the Port, GetNextDevice() may either return
  EFI_SUCCESS and set PortMultiplierPort to 0xFFFF or return EFI_NOT_FOUND (in which case the
  PortMultiplierPort value is undefined).

  @param[in]     This                  The PPI instance pointer.
  @param[in]     Port                  The port number present on the ATA controller.
  @param[in,out] PortMultiplierPort    On input, a pointer to the port multiplier
                                       port number of an ATA device present on the
                                       ATA controller. If on input a PortMultiplierPort
                                       of 0xFFFF is specified, then the port multiplier
                                       port number of the first ATA device is returned.
                                       On output, a pointer to the port multiplier port
                                       number of the next ATA device present on an ATA
                                       controller.

  @retval EFI_SUCCESS              The port multiplier port number of the next ATA
                                   device on the port of the ATA controller was
                                   returned in PortMultiplierPort.
  @retval EFI_NOT_FOUND            There are no more ATA devices on this port of
                                   the ATA controller.
  @retval EFI_INVALID_PARAMETER    PortMultiplierPort is not 0xFFFF, and PortMultiplierPort
                                   was not returned on a previous call to GetNextDevice().

**/
EFI_STATUS
EFIAPI
AhciAtaPassThruGetNextDevice (
  IN     EDKII_PEI_ATA_PASS_THRU_PPI  *This,
  IN     UINT16                       Port,
  IN OUT UINT16                       *PortMultiplierPort
  )
{
  PEI_AHCI_CONTROLLER_PRIVATE_DATA  *Private;
  PEI_AHCI_ATA_DEVICE_DATA          *DeviceData;
  LIST_ENTRY                        *Node;

  if ((This == NULL) || (PortMultiplierPort == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = GET_AHCI_PEIM_HC_PRIVATE_DATA_FROM_THIS_PASS_THRU (This);

  if (Private->PreviousPortMultiplier == 0xFFFF) {
    //
    // If a device is directly attached on a port, previous call to this
    // function will return the value 0xFFFF for PortMultiplierPort. In
    // this case, there should be no more device on the port multiplier.
    //
    Private->PreviousPortMultiplier = 0;
    return EFI_NOT_FOUND;
  }

  if (*PortMultiplierPort == Private->PreviousPortMultiplier) {
    Node = GetFirstNode (&Private->DeviceList);

    while (!IsNull (&Private->DeviceList, Node)) {
      DeviceData = AHCI_PEI_ATA_DEVICE_INFO_FROM_THIS (Node);

      if ((DeviceData->Port == Port) &&
          (DeviceData->PortMultiplier > *PortMultiplierPort))
      {
        *PortMultiplierPort = DeviceData->PortMultiplier;
        goto Exit;
      }

      Node = GetNextNode (&Private->DeviceList, Node);
    }

    return EFI_NOT_FOUND;
  } else if (*PortMultiplierPort == 0xFFFF) {
    //
    // If the PortMultiplierPort is all 0xFF's, start to traverse the device list
    // from the beginning.
    //
    Node = GetFirstNode (&Private->DeviceList);

    while (!IsNull (&Private->DeviceList, Node)) {
      DeviceData = AHCI_PEI_ATA_DEVICE_INFO_FROM_THIS (Node);

      if (DeviceData->Port == Port) {
        *PortMultiplierPort = DeviceData->PortMultiplier;
        goto Exit;
      }

      Node = GetNextNode (&Private->DeviceList, Node);
    }

    return EFI_NOT_FOUND;
  } else {
    //
    // PortMultiplierPort is not equal to all 0xFF's and not equal to previous
    // return value.
    //
    return EFI_INVALID_PARAMETER;
  }

Exit:
  //
  // Update the PreviousPortMultiplier.
  //
  Private->PreviousPortMultiplier = *PortMultiplierPort;

  return EFI_SUCCESS;
}

/**
  Gets the device path information of the underlying ATA host controller.

  @param[in]  This                The PPI instance pointer.
  @param[out] DevicePathLength    The length of the device path in bytes specified
                                  by DevicePath.
  @param[out] DevicePath          The device path of the underlying ATA host controller.
                                  This field re-uses EFI Device Path Protocol as
                                  defined by Section 10.2 EFI Device Path Protocol
                                  of UEFI 2.7 Specification.

  @retval EFI_SUCCESS              The device path of the ATA host controller has
                                   been successfully returned.
  @retval EFI_INVALID_PARAMETER    DevicePathLength or DevicePath is NULL.
  @retval EFI_OUT_OF_RESOURCES     Not enough resource to return the device path.

**/
EFI_STATUS
EFIAPI
AhciAtaPassThruGetDevicePath (
  IN  EDKII_PEI_ATA_PASS_THRU_PPI  *This,
  OUT UINTN                        *DevicePathLength,
  OUT EFI_DEVICE_PATH_PROTOCOL     **DevicePath
  )
{
  PEI_AHCI_CONTROLLER_PRIVATE_DATA  *Private;

  if ((This == NULL) || (DevicePathLength == NULL) || (DevicePath == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = GET_AHCI_PEIM_HC_PRIVATE_DATA_FROM_THIS_PASS_THRU (This);

  *DevicePathLength = Private->DevicePathLength;
  *DevicePath       = AllocateCopyPool (Private->DevicePathLength, Private->DevicePath);
  if (*DevicePath == NULL) {
    *DevicePathLength = 0;
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
