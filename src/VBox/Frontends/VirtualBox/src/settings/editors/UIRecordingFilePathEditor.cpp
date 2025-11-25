/* $Id$ */
/** @file
 * VBox Qt GUI - UIRecordingFilePathEditor class implementation.
 */

/*
 * Copyright (C) 2006-2025 Oracle and/or its affiliates.
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

/* Qt includes: */
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>

/* GUI includes: */
#include "QIAdvancedSlider.h"
#include "UICommon.h"
#include "UIConverter.h"
#include "UIFilePathSelector.h"
#include "UIFilmContainer.h"
#include "UIGlobalSession.h"
#include "UIRecordingFilePathEditor.h"

/* COM includes: */
#include "CSystemProperties.h"

/* Defines: */
#define VIDEO_CAPTURE_BIT_RATE_MIN 32
#define VIDEO_CAPTURE_BIT_RATE_MAX 2048


UIRecordingFilePathEditor::UIRecordingFilePathEditor(QWidget *pParent /* = 0 */, bool fShowInBasicMode /* = false */)
    : UIEditor(pParent, fShowInBasicMode /* show in basic mode */)
    , m_pLabelFilePath(0)
    , m_pEditorFilePath(0)
{
    prepare();
}

void UIRecordingFilePathEditor::setFolder(const QString &strFolder)
{
    /* Update cached value and
     * file editor if value has changed: */
    if (m_strFolder != strFolder)
    {
        m_strFolder = strFolder;
        if (m_pEditorFilePath)
            m_pEditorFilePath->setInitialPath(m_strFolder);
    }
}

QString UIRecordingFilePathEditor::folder() const
{
    return m_pEditorFilePath ? m_pEditorFilePath->initialPath() : m_strFolder;
}

void UIRecordingFilePathEditor::setFilePath(const QString &strFilePath)
{
    /* Update cached value and
     * file editor if value has changed: */
    if (m_strFilePath != strFilePath)
    {
        m_strFilePath = strFilePath;
        if (m_pEditorFilePath)
            m_pEditorFilePath->setPath(m_strFilePath);
    }
}

QString UIRecordingFilePathEditor::filePath() const
{
    return m_pEditorFilePath ? m_pEditorFilePath->path() : m_strFilePath;
}

void UIRecordingFilePathEditor::sltRetranslateUI()
{
    m_pLabelFilePath->setText(tr("File &Path"));
    m_pEditorFilePath->setToolTip(tr("The filename VirtualBox uses to save the recorded content"));
}

void UIRecordingFilePathEditor::prepare()
{
    /* Prepare everything: */
    prepareWidgets();
    prepareConnections();

    /* Apply language settings: */
    sltRetranslateUI();
}

void UIRecordingFilePathEditor::prepareWidgets()
{
    /* Prepare main layout: */
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    if (pLayout)
    {
        pLayout->setContentsMargins(0, 0, 0, 0);


        /* Prepare recording file path label: */
        m_pLabelFilePath = new QLabel(this);
        if (m_pLabelFilePath)
        {
            m_pLabelFilePath->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLayout->addWidget(m_pLabelFilePath);
            pLayout->setAlignment(m_pLabelFilePath, Qt::AlignLeft);
        }

        /* Prepare recording file path editor: */
        m_pEditorFilePath = new UIFilePathSelector(this);
        if (m_pEditorFilePath)
        {
            if (m_pLabelFilePath)
                m_pLabelFilePath->setBuddy(m_pEditorFilePath);
            m_pEditorFilePath->setEditable(false);
            m_pEditorFilePath->setMode(UIFilePathSelector::Mode_File_Save);
            m_pEditorFilePath->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
            pLayout->addWidget(m_pEditorFilePath);
        }

    }
}

void UIRecordingFilePathEditor::prepareConnections()
{
}

void UIRecordingFilePathEditor::filterOut(bool fExpertMode, const QString &strFilter, const QMap<QString, QVariant> &flags)
{
    UIEditor::filterOut(fExpertMode, strFilter, flags);
}
