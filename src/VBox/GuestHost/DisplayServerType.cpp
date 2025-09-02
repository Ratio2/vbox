/* $Id$ */
/** @file
 * Guest / Host common code - Session type detection + handling.
 */

/*
 * Copyright (C) 2023-2025 Oracle and/or its affiliates.
 *
 * This file is part of VirtualBox base platform packages, as
 * available from https://www.virtualbox.org.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, in version 3 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */


/*********************************************************************************************************************************
*   Header Files                                                                                                                 *
*********************************************************************************************************************************/
#include <iprt/assert.h>
#include <iprt/env.h>
#include <iprt/string.h>
#include <iprt/ldr.h>
#include <iprt/log.h>

#include <VBox/GuestHost/DisplayServerType.h>

#ifdef RT_OS_SOLARIS
/*
 * For some unknown reason there is no libX11.so.6 on Solaris but
 * libX11.so/libX11.so.5 which both link to libX11.so.4...
 */
# define X11_LIBRARY_NAME "libX11.so"
#else
# define X11_LIBRARY_NAME "libX11.so.6"
#endif


/*********************************************************************************************************************************
*   Implementation                                                                                                               *
*********************************************************************************************************************************/

/**
 * Returns the VBGHDISPLAYSERVERTYPE as a string.
 *
 * @returns VBGHDISPLAYSERVERTYPE as a string.
 * @param   enmType             VBGHDISPLAYSERVERTYPE to return as a string.
 */
const char *VBGHDisplayServerTypeToStr(VBGHDISPLAYSERVERTYPE enmType)
{
    switch (enmType)
    {
        RT_CASE_RET_STR(VBGHDISPLAYSERVERTYPE_NONE);
        RT_CASE_RET_STR(VBGHDISPLAYSERVERTYPE_AUTO);
        RT_CASE_RET_STR(VBGHDISPLAYSERVERTYPE_X11);
        RT_CASE_RET_STR(VBGHDISPLAYSERVERTYPE_PURE_WAYLAND);
        RT_CASE_RET_STR(VBGHDISPLAYSERVERTYPE_XWAYLAND);
        default: break;
    }

    AssertFailedReturn("<invalid>");
}


#define GET_SYMBOL(a_Mod, a_Name, a_Fn) \
    if (RT_SUCCESS(rc)) \
        rc = RTLdrGetSymbol(a_Mod, a_Name, (void **)&a_Fn); \
    if (RT_FAILURE(rc)) \
        LogRel2(("Symbol '%s' unable to load, rc=%Rrc\n", a_Name, rc));

/**
 * Tries to detect the desktop display server type the process is running in.
 *
 * @returns A value of VBGHDISPLAYSERVERTYPE, or VBGHDISPLAYSERVERTYPE_NONE if detection was not successful.
 *
 * @note    Precedence is:
 *            - Connecting to Wayland (via libwayland-client.so) and/or X11  (via libX11.so).
 *            - VBGH_ENV_WAYLAND_DISPLAY
 *            - VBGH_ENV_XDG_SESSION_TYPE
 *            - VBGH_ENV_XDG_CURRENT_DESKTOP.
 *
 *          Will print a warning to the release log if configuration mismatches.
 */
VBGHDISPLAYSERVERTYPE VBGHDisplayServerTypeDetect(void)
{
    LogRel2(("Detecting display server ...\n"));

    /* Try to connect to the wayland display, assuming it succeeds only when a wayland compositor is active: */
    bool     fHasWayland    = false;
    RTLDRMOD hWaylandClient = NIL_RTLDRMOD;

    int rc = RTLdrLoadSystem("libwayland-client.so.0", true /*fNoUnload*/, &hWaylandClient);
    if (RT_SUCCESS(rc))
    {
        void * (*pWaylandDisplayConnect)(const char *) = NULL;
        GET_SYMBOL(hWaylandClient, "wl_display_connect", pWaylandDisplayConnect);
        void (*pWaylandDisplayDisconnect)(void *) = NULL;
        GET_SYMBOL(hWaylandClient, "wl_display_disconnect", pWaylandDisplayDisconnect);
        if (RT_SUCCESS(rc))
        {
            AssertPtrReturn(pWaylandDisplayConnect, VBGHDISPLAYSERVERTYPE_NONE);
            AssertPtrReturn(pWaylandDisplayDisconnect, VBGHDISPLAYSERVERTYPE_NONE);
            void *pDisplay = pWaylandDisplayConnect(NULL);
            if (pDisplay)
            {
                fHasWayland = true;
                pWaylandDisplayDisconnect(pDisplay);
            }
            else
                LogRel2(("Connecting to Wayland display failed\n"));
        }
        RTLdrClose(hWaylandClient);
    }

    /* Also try to connect to the default X11 display to determine if Xserver is running: */
    bool     fHasX = false;
    RTLDRMOD hX11  = NIL_RTLDRMOD;
    rc = RTLdrLoadSystem(X11_LIBRARY_NAME, true /*fNoUnload*/, &hX11);
    if (RT_SUCCESS(rc))
    {
        void * (*pfnOpenDisplay)(const char *) = NULL;
        GET_SYMBOL(hX11, "XOpenDisplay", pfnOpenDisplay);
        int (*pfnCloseDisplay)(void *) = NULL;
        GET_SYMBOL(hX11, "XCloseDisplay", pfnCloseDisplay);
        if (RT_SUCCESS(rc))
        {
            AssertPtrReturn(pfnOpenDisplay, VBGHDISPLAYSERVERTYPE_NONE);
            AssertPtrReturn(pfnCloseDisplay, VBGHDISPLAYSERVERTYPE_NONE);
            void *pDisplay = pfnOpenDisplay(NULL);
            if (pDisplay)
            {
                fHasX = true;
                pfnCloseDisplay(pDisplay);
            }
            else
                LogRel2(("Opening X display failed\n"));
        }

        RTLdrClose(hX11);
    }

    /* If both wayland and X11 display can be connected then we should have XWayland: */
    VBGHDISPLAYSERVERTYPE retSessionType = VBGHDISPLAYSERVERTYPE_NONE;
    if (fHasWayland && fHasX)
        retSessionType = VBGHDISPLAYSERVERTYPE_XWAYLAND;
    else if (fHasWayland)
        retSessionType = VBGHDISPLAYSERVERTYPE_PURE_WAYLAND;
    else if (fHasX)
        retSessionType = VBGHDISPLAYSERVERTYPE_X11;

    LogRel2(("Detected via connection: %s\n", VBGHDisplayServerTypeToStr(retSessionType)));

    return retSessionType;
}

/**
 * Detect GTK library.
 *
 * @returns \c true if GTK library is available in the system.
 */
bool VBGHDisplayServerTypeIsGtkAvailable(void)
{
    RTLDRMOD hGtk = NIL_RTLDRMOD;
    void * (*pfnGtkInit)(const char *) = NULL;

    int rc = RTLdrLoadSystem("libgtk-3.so.0", true /*fNoUnload*/, &hGtk);
    if (RT_SUCCESS(rc))
    {
        GET_SYMBOL(hGtk, "gtk_init", pfnGtkInit);
        RTLdrClose(hGtk);
    }

    return RT_SUCCESS(rc) && RT_VALID_PTR(pfnGtkInit);
}

#undef GET_SYMBOL

/**
 * Returns true if @a enmType is indicating running X.
 *
 * @returns \c true if @a enmType is running X, \c false if not.
 * @param   enmType             Type to check.
 */
bool VBGHDisplayServerTypeIsXAvailable(VBGHDISPLAYSERVERTYPE enmType)
{
    return    enmType == VBGHDISPLAYSERVERTYPE_XWAYLAND
           || enmType == VBGHDISPLAYSERVERTYPE_X11;
}

/**
 * Returns true if @a enmType is indicating running Wayland.
 *
 * @returns \c true if @a enmType is running Wayland, \c false if not.
 * @param   enmType             Type to check.
 */
bool VBGHDisplayServerTypeIsWaylandAvailable(VBGHDISPLAYSERVERTYPE enmType)
{
    return    enmType == VBGHDISPLAYSERVERTYPE_XWAYLAND
           || enmType == VBGHDISPLAYSERVERTYPE_PURE_WAYLAND;
}

