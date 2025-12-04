/* $Id$ */
/** @file
 * VBox Qt GUI - UIRecordingScreenSelectorEditor class implementation.
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
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

/* GUI includes: */
#include "UIConverter.h"
#include "UIFilmContainer.h"
#include "UIGlobalSession.h"
#include "UIRecordingAudioProfileEditor.h"
#include "UIRecordingScreenSelectorEditor.h"
#include "UIRecordingFilePathEditor.h"
#include "UIRecordingVideoBitrateEditor.h"
#include "UIRecordingVideoFrameRateEditor.h"
#include "UIRecordingVideoFrameSizeEditor.h"

/* COM includes: */
#include "KRecordingFeature.h"

UIRecordingScreenSelectorEditor::UIRecordingScreenSelectorEditor(QWidget *pParent /* = 0 */, bool fShowInBasicMode /* = false */)
    : UIEditor(pParent, fShowInBasicMode)
    , m_pLayout(0)
    , m_pLabel(0)
    , m_pScroller(0)
{
    prepare();
}

int UIRecordingScreenSelectorEditor::minimumLabelHorizontalHint() const
{
    return m_pLabel ? m_pLabel->minimumSizeHint().width() : 0;
}

void UIRecordingScreenSelectorEditor::setMinimumLayoutIndent(int iIndent)
{
    if (m_pLayout)
        m_pLayout->setColumnMinimumWidth(0, iIndent + m_pLayout->spacing());
}

void UIRecordingScreenSelectorEditor::setScreens(const QVector<bool> &screens)
{
    if (m_pScroller && m_pScroller->value() != screens)
        m_pScroller->setValue(screens);
}

QVector<bool> UIRecordingScreenSelectorEditor::screens() const
{
    return m_pScroller ? m_pScroller->value() : QVector<bool>();
}

void UIRecordingScreenSelectorEditor::sltRetranslateUI()
{
    m_pLabel->setText(tr("Scree&ns"));
}

void UIRecordingScreenSelectorEditor::prepare()
{
    /* Prepare everything: */
    prepareWidgets();

    /* Apply language settings: */
    sltRetranslateUI();
}

void UIRecordingScreenSelectorEditor::prepareWidgets()
{
    /* Prepare main layout: */
    m_pLayout = new QGridLayout(this);
    if (m_pLayout)
    {
        m_pLayout->setContentsMargins(0, 0, 0, 0);

        /* Prepare recording screens label: */
        m_pLabel = new QLabel(this);
        if (m_pLabel)
        {
            m_pLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
            m_pLayout->addWidget(m_pLabel, 0, 0);
        }
        /* Prepare recording screens scroller: */
        m_pScroller = new UIFilmContainer(this);
        if (m_pScroller)
        {
            if (m_pLabel)
                m_pLabel->setBuddy(m_pScroller);
            m_pLayout->addWidget(m_pScroller, 0, 1, 1, 5);
        }
    }
}
