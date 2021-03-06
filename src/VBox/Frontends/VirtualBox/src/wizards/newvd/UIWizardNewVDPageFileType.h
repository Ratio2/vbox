/* $Id$ */
/** @file
 * VBox Qt GUI - UIWizardNewVDPageFileType class declaration.
 */

/*
 * Copyright (C) 2006-2020 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifndef FEQT_INCLUDED_SRC_wizards_newvd_UIWizardNewVDPageFileType_h
#define FEQT_INCLUDED_SRC_wizards_newvd_UIWizardNewVDPageFileType_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

/* GUI includes: */
#include "UIWizardPage.h"

/* COM includes: */
#include "COMEnums.h"
#include "CMediumFormat.h"

/* Forward declarations: */
class QVBoxLayout;
class QButtonGroup;
class QRadioButton;
class QIRichTextLabel;


/* 1st page of the New Virtual Hard Drive wizard (base part): */
class SHARED_LIBRARY_STUFF UIWizardNewVDPageBaseFileType : public UIWizardPageBase
{
protected:

    /* Constructor: */
    UIWizardNewVDPageBaseFileType();

    /* Helping stuff: */
    void addFormatButton(QWidget *pParent, QVBoxLayout *pFormatsLayout, CMediumFormat medFormat, bool fPreferred = false);

    QWidget *createFormatButtonGroup(bool fExperMode);

    /* Stuff for 'mediumFormat' field: */
    CMediumFormat mediumFormat() const;
    void setMediumFormat(const CMediumFormat &mediumFormat);
    void retranslateWidgets();

    /* Variables: */
    QButtonGroup *m_pFormatButtonGroup;
    QList<CMediumFormat> m_formats;
    QStringList m_formatNames;
    QStringList m_formatExtensions;
};


/* 1st page of the New Virtual Hard Drive wizard (basic extension): */
class SHARED_LIBRARY_STUFF UIWizardNewVDPageFileType : public UIWizardPage, public UIWizardNewVDPageBaseFileType
{
    Q_OBJECT;
    Q_PROPERTY(CMediumFormat mediumFormat READ mediumFormat WRITE setMediumFormat);

public:

    /* Constructor: */
    UIWizardNewVDPageFileType();

private:

    /* Translation stuff: */
    void retranslateUi();

    /* Prepare stuff: */
    void initializePage();

    /* Validation stuff: */
    bool isComplete() const;

    /* Navigation stuff: */
    int nextId() const;

    /* Widgets: */
    QIRichTextLabel *m_pLabel;
};


#endif /* !FEQT_INCLUDED_SRC_wizards_newvd_UIWizardNewVDPageFileType_h */
