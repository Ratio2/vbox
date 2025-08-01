/* $Id$ */
/** @file
 * VBoxWinDrvInst - Header for Windows driver installation handling.
 */

/*
 * Copyright (C) 2024 Oracle and/or its affiliates.
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
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL), a copy of it is provided in the "COPYING.CDDL" file included
 * in the VirtualBox distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR CDDL-1.0
 */

#ifndef VBOX_INCLUDED_GuestHost_VBoxWinDrvInst_h
#define VBOX_INCLUDED_GuestHost_VBoxWinDrvInst_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <iprt/types.h>
#include <iprt/win/windows.h>

RT_C_DECLS_BEGIN

/** @defgroup grp_vboxwindrvinst    Windows driver / service (un)installation and management functions.
 * @{
 */

/** Windows driver installer handle. */
typedef R3PTRTYPE(struct VBOXWINDRVINSTINTERNAL *) VBOXWINDRVINST;
/** Pointer to a Windows driver installer handle. */
typedef VBOXWINDRVINST                            *PVBOXWINDRVINST;
/** Nil Windows driver installer handle. */
#define NIL_VBOXWINDRVINST                         ((VBOXWINDRVINST)0)

/**
 * Enumeration for the Windows driver installation logging type.
 *
 * Used by the log message callback.
 */
typedef enum VBOXWINDRIVERLOGTYPE
{
    VBOXWINDRIVERLOGTYPE_INVALID = 0,
    VBOXWINDRIVERLOGTYPE_INFO,
    VBOXWINDRIVERLOGTYPE_VERBOSE,
    VBOXWINDRIVERLOGTYPE_WARN,
    VBOXWINDRIVERLOGTYPE_ERROR,
    /** If the (un)installation indicates that a system reboot is required. */
    VBOXWINDRIVERLOGTYPE_REBOOT_NEEDED
} VBOXWINDRIVERLOGTYPE;

/**
 * Log message callback.
 *
 * @param   enmType     Log type.
 * @param   pszMsg      Log message.
 * @param   pvUser      User-supplied pointer. Might be NULL if not being used.
 */
typedef DECLCALLBACKTYPE(void, FNVBOXWINDRIVERLOGMSG,(VBOXWINDRIVERLOGTYPE enmType, const char *pszMsg, void *pvUser));
/** Pointer to message callback function. */
typedef FNVBOXWINDRIVERLOGMSG *PFNVBOXWINDRIVERLOGMSG;

/** No flags specified. */
#define VBOX_WIN_DRIVERINSTALL_F_NONE       0
/** Try a silent installation (if possible).
 *
 *  When having this flag set, this will result in an ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED error
 *  if drivers get installed with our mixed SHA1 / SH256 certificates on older Windows OSes (7, Vista, ++).
 *
 *  However, if VBOX_WIN_DRIVERINSTALL_F_SILENT is missing, this will result in a
 *  (desired) Windows driver installation dialog to confirm (or reject) the installation
 *  by the user.
 *
 *  On the other hand, for unattended installs we need VBOX_WIN_DRIVERINSTALL_F_SILENT
 *  being set, as our certificates will get installed into the Windows certificate
 *  store *before* we perform any driver installation.
 *
 *  So be careful using this flag to not break installations.
 */
#define VBOX_WIN_DRIVERINSTALL_F_SILENT     RT_BIT(0)
/** Force driver installation, even if a newer driver version already is installed (overwrite). */
#define VBOX_WIN_DRIVERINSTALL_F_FORCE      RT_BIT(1)
/** Run in dry mode (no real (un)installation performed). */
#define VBOX_WIN_DRIVERINSTALL_F_DRYRUN     RT_BIT(2)
/** Do not destroy internal data for later inspection.
 *  Only used by testcases and should be avoided in general. */
#define VBOX_WIN_DRIVERINSTALL_F_NO_DESTROY RT_BIT(3)
/** Validation mask. */
#define VBOX_WIN_DRIVERINSTALL_F_VALID_MASK 0xf

/**
 * Enumeration for Windows driver service functions.
 */
typedef enum VBOXWINDRVSVCFN
{
    /** Invalid function. */
    VBOXWINDRVSVCFN_INVALID = 0,
    /** Starts the service. */
    VBOXWINDRVSVCFN_START,
    /** Stops the service. */
    VBOXWINDRVSVCFN_STOP,
    /** Restart the service. */
    VBOXWINDRVSVCFN_RESTART,
    /** Deletes a service. */
    VBOXWINDRVSVCFN_DELETE,
    /** End marker, do not use. */
    VBOXWINDRVSVCFN_END
} VBOXWINDRVSVCFN;

/** No service function flags specified. */
#define VBOXWINDRVSVCFN_F_NONE              0
/** Wait for the service function to get executed. */
#define VBOXWINDRVSVCFN_F_WAIT              RT_BIT(0)
/** Validation mask. */
#define VBOXWINDRVSVCFN_F_VALID_MASK        0x1

/** Struct for keeping Windows service information. */
typedef struct VBOXWINDRVSVCINFO
{
    /** Holds the  file version (maj.min.build) of the (resolved) binary. */
    char                     szVer[128];
    LPSERVICE_STATUS_PROCESS pStatus;
    LPQUERY_SERVICE_CONFIGW  pConfig;
} VBOXWINDRVSVCINFO;
/** Pointer to a struct for keeping Windows service information. */
typedef VBOXWINDRVSVCINFO *PVBOXWINDRVSVCINFO;

/**
 * Pattern handling callback.
 *
 * @return  The resolved pattern if any, or NULL if not being handled.
 * @param   pszPattern          Pattern which got matched.
 * @param   pvUser              User-supplied pointer. Might be NULL if not being used.
 */
typedef DECLCALLBACKTYPE(char *, FNVBOXWINDRVSTRPATTERN,(const char *pszPattern, void *pvUser));
/** Pointer to pattern handling callback. */
typedef FNVBOXWINDRVSTRPATTERN *PFVBOXWINDRVSTRPATTERN;

/**
 * Struct for keeping a Windows Driver installation pattern match entry.
 */
typedef struct
{
    /** Pattern to match.
     *  No wildcards supported (yet). */
    const char            *psz;
    /** Pattern replacement function to invoke.
     *  If NULL, the matched pattern will be removed from the output. */
    PFVBOXWINDRVSTRPATTERN pfn;
    /** User-supplied pointer. Optional and can be NULL. */
    void                  *pvUser;
    /** Where to store the pattern replacement on success.
     *  Only used internally and must not be used. */
    char                  *rep;
} VBOXWINDRVSTRPATTERN;
/** Pointer to a struct for keeping a Windows Driver installation pattern match entry. */
typedef VBOXWINDRVSTRPATTERN *PVBOXWINDRVSTRPATTERN;

/** @defgroup grp_windrvinst_svc     Installation / uninstallation functions
 * @{
 */
int VBoxWinDrvInstCreate(PVBOXWINDRVINST hDrvInst);
int VBoxWinDrvInstCreateEx(PVBOXWINDRVINST phDrvInst, unsigned uVerbosity, PFNVBOXWINDRIVERLOGMSG pfnLog, void *pvUser);
int VBoxWinDrvInstDestroy(VBOXWINDRVINST hDrvInst);
unsigned VBoxWinDrvInstGetWarnings(VBOXWINDRVINST hDrvInst);
unsigned VBoxWinDrvInstGetErrors(VBOXWINDRVINST hDrvInst);
void VBoxWinDrvInstSetOsVersion(VBOXWINDRVINST hDrvInst, uint64_t uOsVer);
unsigned VBoxWinDrvInstSetVerbosity(VBOXWINDRVINST hDrvInst, uint8_t uVerbosity);
void VBoxWinDrvInstSetLogCallback(VBOXWINDRVINST hDrvInst, PFNVBOXWINDRIVERLOGMSG pfnLog, void *pvUser);
int VBoxWinDrvInstInstallEx(VBOXWINDRVINST hDrvInst, const char *pszInfFile, const char *pszModel, const char *pszPnpId, uint32_t fFlags);
int VBoxWinDrvInstInstall(VBOXWINDRVINST hDrvInst, const char *pszInfFile, uint32_t fFlags);
int VBoxWinDrvInstInstallExecuteInf(VBOXWINDRVINST hDrvInst, const char *pszInfFile, const char *pszSection, uint32_t fFlags);
int VBoxWinDrvInstUninstall(VBOXWINDRVINST hDrvInst, const char *pszInfFile, const char *pszModel, const char *pszPnPId, uint32_t fFlags);
int VBoxWinDrvInstUninstallExecuteInf(VBOXWINDRVINST hDrvInst, const char *pszInfFile, const char *pszSection, uint32_t fFlags);
/** @} */

/** @name Native NT functions.
 * @{
 */
int VBoxWinDrvInstQueryNtLinkTarget(PCRTUTF16 pwszLinkNt, PRTUTF16 *ppwszLinkTarget);
/** @} */

/** @name Service functions
 * @{
 */
int VBoxWinDrvInstServiceControl(VBOXWINDRVINST hDrvInst, const char *pszService, VBOXWINDRVSVCFN enmFn);
int VBoxWinDrvInstServiceControlEx(VBOXWINDRVINST hDrvInst, const char *pszService, VBOXWINDRVSVCFN enmFn, uint32_t fFlags, RTMSINTERVAL msTimeout);
int VBoxWinDrvInstServiceQuery(const char *pszService, PVBOXWINDRVSVCINFO pSvcInfo);
void VBoxWinDrvInstServiceInfoDestroy(PVBOXWINDRVSVCINFO pSvcInfo);
/** @} */

/** @name String functions
 * @{
 */
int VBoxWinDrvPatternReplace(const char *pszInput, const PVBOXWINDRVSTRPATTERN paPatterns, size_t cPatterns, char **ppszOutput);
/** @} */

/** @name File functions
 * @{
 */
int VBoxWinDrvInstFileQueryVersionEx(const char *pszPath, uint32_t *puMajor, uint32_t *puMinor, uint32_t *puBuildNumber, uint32_t *puRevisionNumber);
int VBoxWinDrvInstFileQueryVersion(const char *pszPath, char *pszVersion, size_t cbVersion);
int VBoxWinDrvInstFileQueryVersionUtf16(PCRTUTF16 pwszPath, char *pszVersion, size_t cbVersion);
/** @} */

/** @name Log functions
 * @{
 */
int VBoxWinDrvInstLogSetupAPI(VBOXWINDRVINST hDrvInst, unsigned cLastSections);
/** @} */

/** @} */

RT_C_DECLS_END

#endif /* !VBOX_INCLUDED_GuestHost_VBoxWinDrvInst_h */

