/* $Id$ */
/** @file
 * VBox Qt GUI - UIWizardNewVMPageHardware class implementation.
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

/* Qt includes: */
#include <QCheckBox>
#include <QGridLayout>
#include <QMetaType>
#include <QVBoxLayout>

/* GUI includes: */
#include "QIRichTextLabel.h"
#include "UIBaseMemoryEditor.h"
#include "UIMessageCenter.h"
#include "UIVirtualCPUEditor.h"
#include "UIWizardNewVM.h"
#include "UIWizardNewVMPageHardware.h"
#include "UIWizardNewVDPageSizeLocation.h"

/* COM includes: */
#include "CGuestOSType.h"

UIWizardNewVMPageHardwareBase::UIWizardNewVMPageHardwareBase()
    : m_pBaseMemoryEditor(0)
    , m_pVirtualCPUEditor(0)
    , m_pEFICheckBox(0)
{
}

int UIWizardNewVMPageHardwareBase::baseMemory() const
{
    if (!m_pBaseMemoryEditor)
        return 0;
    return m_pBaseMemoryEditor->value();
}

int UIWizardNewVMPageHardwareBase::VCPUCount() const
{
    if (!m_pVirtualCPUEditor)
        return 1;
    return m_pVirtualCPUEditor->value();
}

bool UIWizardNewVMPageHardwareBase::EFIEnabled() const
{
    if (!m_pEFICheckBox)
        return false;
    return m_pEFICheckBox->isChecked();
}

void UIWizardNewVMPageHardwareBase::retranslateWidgets()
{
    if (m_pEFICheckBox)
    {
        m_pEFICheckBox->setText(UIWizardNewVM::tr("&Enable EFI (special OSes only)"));
        m_pEFICheckBox->setToolTip(UIWizardNewVM::tr("<p>When checked, the guest will support the "
                                                       "Extended Firmware Interface (EFI), which is required to boot certain "
                                                       "guest OSes. Non-EFI aware OSes will not be able to boot if this option is activated.</p>"));
    }
}

QWidget *UIWizardNewVMPageHardwareBase::createHardwareWidgets()
{
    QWidget *pHardwareContainer = new QWidget;
    QGridLayout *pHardwareLayout = new QGridLayout(pHardwareContainer);
    pHardwareLayout->setContentsMargins(0, 0, 0, 0);

    m_pBaseMemoryEditor = new UIBaseMemoryEditor(0, true);
    m_pVirtualCPUEditor = new UIVirtualCPUEditor(0, true);
    m_pEFICheckBox      = new QCheckBox;
    pHardwareLayout->addWidget(m_pBaseMemoryEditor, 0, 0, 1, 4);
    pHardwareLayout->addWidget(m_pVirtualCPUEditor, 1, 0, 1, 4);
    pHardwareLayout->addWidget(m_pEFICheckBox, 2, 0, 1, 1);

    return pHardwareContainer;
}

UIWizardNewVMPageHardware::UIWizardNewVMPageHardware()
    : m_pLabel(0)
{
    prepare();
    qRegisterMetaType<CMedium>();
    registerField("baseMemory", this, "baseMemory");
    registerField("VCPUCount", this, "VCPUCount");
    registerField("EFIEnabled", this, "EFIEnabled");
}

void UIWizardNewVMPageHardware::prepare()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);

    m_pLabel = new QIRichTextLabel(this);
    pMainLayout->addWidget(m_pLabel);
    pMainLayout->addWidget(createHardwareWidgets());

    pMainLayout->addStretch();
    createConnections();
}

void UIWizardNewVMPageHardware::createConnections()
{
}

void UIWizardNewVMPageHardware::retranslateUi()
{
    setTitle(UIWizardNewVM::tr("Hardware"));

    if (m_pLabel)
        m_pLabel->setText(UIWizardNewVM::tr("<p>You can modify virtual machine's hardware by changing amount of RAM and "
                                            "virtual CPU count. Enabling EFI is also possible.</p>"));

    retranslateWidgets();
}

void UIWizardNewVMPageHardware::initializePage()
{
    retranslateUi();

    if (!field("type").canConvert<CGuestOSType>())
        return;

    CGuestOSType type = field("type").value<CGuestOSType>();
    ULONG recommendedRam = type.GetRecommendedRAM();
    if (m_pBaseMemoryEditor)
        m_pBaseMemoryEditor->setValue(recommendedRam);

    KFirmwareType fwType = type.GetRecommendedFirmware();
    if (m_pEFICheckBox)
        m_pEFICheckBox->setChecked(fwType != KFirmwareType_BIOS);
}

void UIWizardNewVMPageHardware::cleanupPage()
{
    UIWizardPage::cleanupPage();
}

bool UIWizardNewVMPageHardware::isComplete() const
{
    return true;
}
