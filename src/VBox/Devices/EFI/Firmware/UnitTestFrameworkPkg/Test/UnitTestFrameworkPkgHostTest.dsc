## @file
# UnitTestFrameworkPkg DSC file used to build host-based unit tests.
#
# Copyright (c) Microsoft Corporation.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME           = UnitTestFrameworkPkgHostTest
  PLATFORM_GUID           = C7F97D6D-54AC-45A9-8197-CC99B20CC7EC
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/UnitTestFrameworkPkg/HostTest
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = NOOPT
  SKUID_IDENTIFIER        = DEFAULT

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgHost.dsc.inc

[PcdsPatchableInModule]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x1F

[Components]
  #
  # Build HOST_APPLICATIONs that test the SampleUnitTest
  #
  UnitTestFrameworkPkg/Test/UnitTest/Sample/SampleUnitTest/SampleUnitTestHost.inf
  UnitTestFrameworkPkg/Test/GoogleTest/Sample/SampleGoogleTest/SampleGoogleTestHost.inf

  #
  # Build HOST_APPLICATION Libraries
  #
  UnitTestFrameworkPkg/Library/CmockaLib/CmockaLib.inf
  UnitTestFrameworkPkg/Library/FunctionMockLib/FunctionMockLib.inf
  UnitTestFrameworkPkg/Library/GoogleTestLib/GoogleTestLib.inf
  UnitTestFrameworkPkg/Library/Posix/DebugLibPosix/DebugLibPosix.inf
  UnitTestFrameworkPkg/Library/Posix/MemoryAllocationLibPosix/MemoryAllocationLibPosix.inf
  UnitTestFrameworkPkg/Library/SubhookLib/SubhookLib.inf
  UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLibCmocka.inf
  UnitTestFrameworkPkg/Library/UnitTestDebugAssertLib/UnitTestDebugAssertLibHost.inf
