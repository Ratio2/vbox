# Copyright © 2012 Intel Corporation
# Copyright © 2014 Emil Velikov
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/mapi \
	-I$(top_srcdir)/src/mesa \
	-I$(top_builddir)/src/util \
	-I$(top_srcdir)/src/mesa/drivers/dri/common \
	$(GALLIUM_CFLAGS) \
	$(LIBDRM_CFLAGS) \
	$(VISIBILITY_CFLAGS)

if HAVE_GALLIUM_SOFTPIPE
AM_CPPFLAGS += \
	-DGALLIUM_SOFTPIPE
endif # HAVE_GALLIUM_SOFTPIPE

noinst_LTLIBRARIES = libdri.la
libdri_la_SOURCES = $(common_SOURCES)

if HAVE_DRISW
libdri_la_SOURCES += $(drisw_SOURCES)
endif


if HAVE_DRI2
libdri_la_SOURCES += $(dri2_SOURCES)
endif

EXTRA_DIST = SConscript
