## @file
# GNU/Linux makefile for 'DevicePath' module build.
#
# Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
ARCH ?= IA32
MAKEROOT ?= ..

APPNAME = DevicePath

OBJECTS = DevicePath.o UefiDevicePathLib.o DevicePathFromText.o  DevicePathUtilities.o

include $(MAKEROOT)/Makefiles/app.makefile

GCCVERSION = $(shell $(CC) -dumpversion | awk -F'.' '{print $$1}')
CLANG := $(shell $(CC) --version | grep clang)
ifneq ("$(GCCVERSION)", "5")
ifeq ($(CLANG),)
ifneq ($(DARWIN),Darwin)
# gcc 12 trips over device path handling
CFLAGS += -Wno-error=stringop-overflow
endif
endif
endif

LIBS = -lCommon
ifeq ($(CYGWIN), CYGWIN)
  LIBS += -L/lib/e2fsprogs -luuid
endif

ifeq ($(LINUX), Linux)
ifndef CROSS_LIB_UUID
  LIBS += -luuid
else
  LIBS += -L$(CROSS_LIB_UUID)
endif
endif

