/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2014 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#define RG_MODULE_STRING "[MIDIInstrumentParameterPanel]"

#include "MIDIInstrumentParameterPanel.h"

#include "gui/widgets/SqueezedLabel.h"
#include "sound/Midi.h"
#include "misc/Debug.h"
#include "misc/Strings.h"
#include "base/AllocateChannels.h"
#include "base/Colour.h"
#include "base/Composition.h"
#include "base/ControlParameter.h"
#include "base/Instrument.h"
#include "base/MidiDevice.h"
#include "base/MidiProgram.h"
#include "document/RosegardenDocument.h"
#include "gui/studio/StudioControl.h"
#include "gui/widgets/Rotary.h"
#include "InstrumentParameterPanel.h"
#include "sequencer/RosegardenSequencer.h"

#include <algorithm>

#include <QComboBox>
#include <QCheckBox>
#include <QColor>
#include <QFontMetrics>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QRegExp>
#include <QSignalMapper>
#include <QSpinBox>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QToolTip>


namespace Rosegarden
{

MIDIInstrumentParameterPanel::MIDIInstrumentParameterPanel(RosegardenDocument *doc, QWidget* parent):
        InstrumentParameterPanel(doc, parent),
        m_rotaryFrame(0),
        m_rotaryMapper(new QSignalMapper(this))
{
    setObjectName("MIDI Instrument Parameter Panel");

    QFont f;
    f.setPointSize(f.pointSize() * 90 / 100);
    f.setBold(false);

    QFontMetrics metrics(f);
    int width25 = metrics.width("1234567890123456789012345");

    m_instrumentLabel->setFont(f);
    m_instrumentLabel->setFixedWidth(width25);
    m_instrumentLabel->setAlignment(Qt::AlignCenter);

    setContentsMargins(2, 2, 2, 2);
    m_mainGrid = new QGridLayout(this);
    m_mainGrid->setMargin(0);
    m_mainGrid->setSpacing(1);
    setLayout(m_mainGrid);

    m_connectionLabel = new SqueezedLabel(this);
    m_bankValue = new QComboBox(this);
    m_programValue = new QComboBox(this);
    m_variationValue = new QComboBox(this);
    m_bankCheckBox = new QCheckBox(this);
    m_programCheckBox = new QCheckBox(this);
    m_variationCheckBox = new QCheckBox(this);
    m_percussionCheckBox = new QCheckBox(this);
    m_channelUsed = new QComboBox(this);

    // Everything else sets up elsewhere, but these don't vary per instrument:
    m_channelUsed->addItem(tr("auto"));
    m_channelUsed->addItem(tr("fixed"));

    m_connectionLabel->setFont(f);
    m_bankValue->setFont(f);
    m_programValue->setFont(f);
    m_variationValue->setFont(f);
    m_bankCheckBox->setFont(f);
    m_programCheckBox->setFont(f);
    m_variationCheckBox->setFont(f);
    m_percussionCheckBox->setFont(f);
    m_channelUsed->setFont(f);

    m_bankValue->setToolTip(tr("<qt>Set the MIDI bank from which to select programs</qt>"));
    m_programValue->setToolTip(tr("<qt>Set the MIDI program or &quot;patch&quot;</p></qt>"));
    m_variationValue->setToolTip(tr("<qt>Set variations on the program above, if available in the studio</qt>"));
    m_percussionCheckBox->setToolTip(tr("<qt><p>Check this to tell Rosegarden that this is a percussion instrument.  This allows you access to any percussion key maps and drum kits you may have configured in the studio</p></qt>"));
    m_channelUsed->setToolTip(tr("<qt><p><i>Auto</i>, allocate channel automatically; <i>Fixed</i>, fix channel to instrument number</p></qt>"));

    m_bankValue->setMaxVisibleItems(20);
    m_programValue->setMaxVisibleItems(20);
    m_variationValue->setMaxVisibleItems(20);
    m_channelUsed->setMaxVisibleItems(2);
    
    m_bankLabel = new QLabel(tr("Bank"), this);
    m_variationLabel = new QLabel(tr("Variation"), this);
    m_programLabel = new QLabel(tr("Program"), this);
    QLabel *percussionLabel = new QLabel(tr("Percussion"), this);
    QLabel *channelLabel = new QLabel(tr("Channel"), this);
    
    m_bankLabel->setFont(f);
    m_variationLabel->setFont(f);
    m_programLabel->setFont(f);
    percussionLabel->setFont(f);
    channelLabel->setFont(f);
    
    // Ensure a reasonable amount of space in the program dropdowns even
    // if no instrument initially selected

    // setMinimumWidth() using QFontMetrics wasn't cutting it at all, so let's
    // try what I used in the plugin manager dialog, with
    // setMinimumContentsLength() instead:
    QString metric("Acoustic Grand Piano #42B");
    int width22 = metric.size();
    
    m_bankValue->setMinimumContentsLength(width22);
    m_programValue->setMinimumContentsLength(width22);
    m_variationValue->setMinimumContentsLength(width22);
    m_channelUsed->setMinimumContentsLength(width22);

    // we still have to use the QFontMetrics here, or a SqueezedLabel will
    // squeeze itself down to 0.
    int width30 = metrics.width("123456789012345678901234567890");
    m_connectionLabel->setFixedWidth(width30);
    m_connectionLabel->setAlignment(Qt::AlignCenter);
    
    
    QString programTip = tr("<qt>Use program changes from an external source to manipulate these controls (only valid for the currently-active track) [Shift + P]</qt>");
    m_evalMidiPrgChgCheckBox = new QCheckBox(this); 
    m_evalMidiPrgChgCheckBox->setFont(f);
    m_evalMidiPrgChgLabel = new QLabel(tr("Receive external"), this);
    m_evalMidiPrgChgLabel->setFont(f);
    m_evalMidiPrgChgLabel->setToolTip(programTip);
    
    m_evalMidiPrgChgCheckBox->setDisabled(false);
    m_evalMidiPrgChgCheckBox->setChecked(false);
    m_evalMidiPrgChgCheckBox->setToolTip(programTip);
    m_evalMidiPrgChgCheckBox->setShortcut((QKeySequence)"Shift+P");



    m_mainGrid->setColumnStretch(2, 1);

    m_mainGrid->addWidget(m_instrumentLabel, 0, 0, 1, 4, Qt::AlignCenter);
    m_mainGrid->addWidget(m_connectionLabel, 1, 0, 1, 4, Qt::AlignCenter);

    m_mainGrid->addWidget(percussionLabel, 3, 0, 1, 2, Qt::AlignLeft);
    m_mainGrid->addWidget(m_percussionCheckBox, 3, 3, Qt::AlignLeft);

    m_mainGrid->addWidget(m_bankLabel, 4, 0, Qt::AlignLeft);
    m_mainGrid->addWidget(m_bankCheckBox, 4, 1, Qt::AlignRight);
    m_mainGrid->addWidget(m_bankValue, 4, 2, 1, 2, Qt::AlignRight);

    m_mainGrid->addWidget(m_programLabel, 5, 0, Qt::AlignLeft);
    m_mainGrid->addWidget(m_programCheckBox, 5, 1, Qt::AlignRight);
    m_mainGrid->addWidget(m_programValue, 5, 2, 1, 2, Qt::AlignRight);

    m_mainGrid->addWidget(m_variationLabel, 6, 0);
    m_mainGrid->addWidget(m_variationCheckBox, 6, 1);
    m_mainGrid->addWidget(m_variationValue, 6, 2, 1, 2, Qt::AlignRight);
      
    m_mainGrid->addWidget(channelLabel, 7, 0, Qt::AlignLeft);
    m_mainGrid->addWidget(m_channelUsed, 7, 2, 1, 2, Qt::AlignRight);

    m_mainGrid->addWidget(m_evalMidiPrgChgLabel, 8, 0, 1, 3, Qt::AlignLeft);
    m_mainGrid->addWidget(m_evalMidiPrgChgCheckBox, 8, 3, Qt::AlignLeft);

    // Disable these by default - they are activated by their checkboxes
    //
    m_programValue->setDisabled(true);
    m_bankValue->setDisabled(true);
    m_variationValue->setDisabled(true);

    // Only active if we have an Instrument selected
    //
    m_percussionCheckBox->setDisabled(true);
    m_programCheckBox->setDisabled(true);
    m_bankCheckBox->setDisabled(true);
    m_variationCheckBox->setDisabled(true);

    // Connect up the toggle boxes
    //
    connect(m_percussionCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotTogglePercussion(bool)));

    connect(m_programCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleProgramChange(bool)));

    connect(m_bankCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleBank(bool)));

    connect(m_variationCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleVariation(bool)));
    
    connect(m_evalMidiPrgChgCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleChangeListOnProgChange(bool)) );
    
    
    // Connect activations
    //
    connect(m_bankValue, SIGNAL(activated(int)),
            this, SLOT(slotSelectBank(int)));

    connect(m_variationValue, SIGNAL(activated(int)),
            this, SLOT(slotSelectVariation(int)));

    connect(m_programValue, SIGNAL(activated(int)),
            this, SLOT(slotSelectProgram(int)));
    
    // don't select any of the options in any dropdown
    m_programValue->setCurrentIndex( -1);
    m_bankValue->setCurrentIndex( -1);
    m_variationValue->setCurrentIndex( -1);
    m_channelUsed->setCurrentIndex(-1);

    connect(m_rotaryMapper, SIGNAL(mapped(int)),
            this, SLOT(slotControllerChanged(int)));
    connect(m_channelUsed, SIGNAL(activated(int)),
            this, SLOT(slotSetUseChannel(int)));
}


void MIDIInstrumentParameterPanel::slotToggleChangeListOnProgChange(bool val){
    // used to disable prog-change select-box 
    // (in MIDIInstrumentParameterPanel), if TrackChanged 
    this->m_evalMidiPrgChgCheckBox->setChecked(val);
}


void
MIDIInstrumentParameterPanel::setupForInstrument(Instrument *instrument)
{
    RG_DEBUG << "MIDIInstrumentParameterPanel::setupForInstrument" << endl;

    // In some cases setupForInstrument gets called several times.
    // This shortcuts this activity since only one setup is needed.
    if (m_selectedInstrument == instrument) {
        RG_DEBUG << "MIDIInstrumentParameterPanel::setupForInstrument "
                 << "-- early exit.  instrument didn't change." << endl;
        return;
    }

    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (instrument->getDevice());
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::setupForInstrument:"
        << " No MidiDevice for Instrument "
        << instrument->getId() << endl;
        return ;
    }

    setSelectedInstrument(instrument,
                          instrument->getLocalizedPresentationName());

    // Set Studio Device name
    //
    QString connection(RosegardenSequencer::getInstance()->getConnection(md->getId()));

    if (connection == "") {
        m_connectionLabel->setText(tr("[ %1 ]").arg(tr("No connection")));
    } else {

        // remove trailing "(duplex)", "(read only)", "(write only)" etc
        connection.replace(QRegExp("\\s*\\([^)0-9]+\\)\\s*$"), "");

        QString text = QObject::tr("[ %1 ]").arg(connection);
        /*QString origText(text);

        QFontMetrics metrics(m_connectionLabel->fontMetrics());
        int maxwidth = metrics.width
            ("Program: [X]   Acoustic Grand Piano 123");// kind of arbitrary!

        int hlen = text.length() / 2;
        while (metrics.width(text) > maxwidth && text.length() > 10) {
            --hlen;
            text = origText.left(hlen) + "..." + origText.right(hlen);
        }

        if (text.length() > origText.length() - 7) text = origText;*/
        m_connectionLabel->setText(QObject::tr(text.toStdString().c_str()));
    }

    // Enable all check boxes
    //
    m_percussionCheckBox->setDisabled(false);
    m_programCheckBox->setDisabled(false);
    m_bankCheckBox->setDisabled(false);
    m_variationCheckBox->setDisabled(false);

    // Activate all checkboxes
    //
    
    // Block signals
    m_percussionCheckBox-> blockSignals(true);
    m_programCheckBox->    blockSignals(true);
    m_bankCheckBox->       blockSignals(true);
    m_variationCheckBox->  blockSignals(true);
    
    // Change state
    m_percussionCheckBox->setChecked(instrument->isPercussion());
    m_programCheckBox->setChecked(instrument->sendsProgramChange());
    m_bankCheckBox->setChecked(instrument->sendsBankSelect());
    m_variationCheckBox->setChecked(instrument->sendsBankSelect());

    // Unblock signals
    m_percussionCheckBox-> blockSignals(false);
    m_programCheckBox->    blockSignals(false);
    m_bankCheckBox->       blockSignals(false);
    m_variationCheckBox->  blockSignals(false);

    // Basic parameters
    //
    //
    // Check for program change
    //
    populateBankList();
    populateProgramList();
    populateVariationList();
    populateChannelList();
    
    // Setup the ControlParameters
    //
    setupControllers(md);

    m_mainGrid->setRowStretch(9, 20);

    // Set all the positions by controller number
    //
    for (RotaryMap::iterator it = m_rotaries.begin() ;
            it != m_rotaries.end(); ++it) {
        MidiByte value = 0;

        try {
            value = instrument->getControllerValue(
                        MidiByte(it->first));
        } catch (...) {
            continue;
        }
        setRotaryToValue(it->first, int(value));
    }

}

void
MIDIInstrumentParameterPanel::setupControllers(MidiDevice *md)
{
    QFont f(font());

    if (!m_rotaryFrame) {
        m_rotaryFrame = new QFrame(this);
        m_mainGrid->addWidget(m_rotaryFrame, 10, 0, 1, 3, Qt::AlignHCenter);
        m_rotaryFrame->setContentsMargins(8, 8, 8, 8);
        m_rotaryGrid = new QGridLayout(m_rotaryFrame);
        m_rotaryGrid->setSpacing(1);
        m_rotaryGrid->setMargin(0);
        m_rotaryGrid->addItem(new QSpacerItem(10, 4), 0, 1);
        m_rotaryFrame->setLayout(m_rotaryGrid);
    }

    // To cut down on flicker, we avoid destroying and recreating
    // widgets as far as possible here.  If a label already exists,
    // we just set its text; if a rotary exists, we only replace it
    // if we actually need a different one.

    Composition &comp = m_doc->getComposition();
    ControlList list = md->getControlParameters();

    // sort by IPB position
    //
    std::sort(list.begin(), list.end(),
              ControlParameter::ControlPositionCmp());

    int count = 0;
    RotaryMap::iterator rmi = m_rotaries.begin();

    for (ControlList::iterator it = list.begin();
            it != list.end(); ++it) {
        if (it->getIPBPosition() == -1)
            continue;

        // Get the knob colour (even if it's default, because otherwise it turns
        // black instead of the default color from the map!  it was here the
        // whole time, this simple!)
        //
        QColor knobColour = it->getColourIndex();
        Colour c =
            comp.getGeneralColourMap().getColourByIndex
            (it->getColourIndex());
        knobColour = QColor(c.getRed(), c.getGreen(), c.getBlue());

        Rotary *rotary = 0;

        if (rmi != m_rotaries.end()) {

            // Update the controller number that is associated with the
            // existing rotary widget.

            rmi->first = it->getControllerValue();

            // Update the properties of the existing rotary widget.

            rotary = rmi->second.first;
            int redraw = 0; // 1 -> position, 2 -> all

            if (rotary->getMinValue() != it->getMin()) {
                rotary->setMinimum(it->getMin());
                redraw = 1;
            }
            if (rotary->getMaxValue() != it->getMax()) {
                rotary->setMaximum(it->getMax());
                redraw = 1;
            }
            
            bool isCentered = it->getDefault() == 64;
            if (rotary->getCentered() != isCentered) {
                rotary->setCentered(isCentered);
                redraw = 1;
            }
            if (rotary->getKnobColour() != knobColour) {
                rotary->setKnobColour(knobColour);
                redraw = 2;
            }
            if (redraw == 1 || rotary->getPosition() != it->getDefault()) {
                rotary->setPosition(it->getDefault());
                if (redraw == 1)
                    redraw = 0;
            }
            if (redraw == 2) {
                rotary->repaint();
            }

            // Update the controller name that is associated with
            // with the existing rotary widget.

            QLabel *label = rmi->second.second;
            label->setText(QObject::tr(it->getName().c_str()));

            ++rmi;

        } else {

            QWidget *hbox = new QWidget(m_rotaryFrame);
            QHBoxLayout *hboxLayout = new QHBoxLayout;
            hboxLayout->setSpacing(8);
            hboxLayout->setMargin(0);

            float smallStep = 1.0;

            float bigStep = 5.0;
            if (it->getMax() - it->getMin() < 10)
                bigStep = 1.0;
            else if (it->getMax() - it->getMin() < 20)
                bigStep = 2.0;

            rotary = new Rotary(hbox,
                                it->getMin(),
                                it->getMax(),
                                smallStep,
                                bigStep,
                                it->getDefault(),
                                20,
                                Rotary::NoTicks,
                                false,
                                it->getDefault() == 64); //!!! hacky

            hboxLayout->addWidget(rotary);
            hbox->setLayout(hboxLayout);

            rotary->setKnobColour(knobColour);

            // Add a label
            QLabel *label = new SqueezedLabel(QObject::tr(it->getName().c_str()), hbox);
            label->setFont(f);
            hboxLayout->addWidget(label);

            RG_DEBUG << "Adding new widget at " << (count / 2) << "," << (count % 2) << endl;

            // Add the compound widget
            //
            m_rotaryGrid->addWidget(hbox, count / 2, (count % 2) * 2, Qt::AlignLeft);
            hbox->show();

            // Add to list
            //
            m_rotaries.push_back(std::pair<int, RotaryPair>
                                 (it->getControllerValue(),
                                  RotaryPair(rotary, label)));

            // Connect
            //
            connect(rotary, SIGNAL(valueChanged(float)),
                    m_rotaryMapper, SLOT(map()));

            rmi = m_rotaries.end();
        }

        // Add signal mapping
        //
        m_rotaryMapper->setMapping(rotary,
                                   int(it->getControllerValue()));

        count++;
    }

    if (rmi != m_rotaries.end()) {
        for (RotaryMap::iterator rmj = rmi; rmj != m_rotaries.end(); ++rmj) {
            delete rmj->second.first;
            delete rmj->second.second;
        }
        m_rotaries = std::vector<std::pair<int, RotaryPair> >
                     (m_rotaries.begin(), rmi);
    }

//    m_rotaryFrame->show();
}

void
MIDIInstrumentParameterPanel::setRotaryToValue(int controller, int value)
{
    /*
    RG_DEBUG << "MIDIInstrumentParameterPanel::setRotaryToValue - "
             << "controller = " << controller
             << ", value = " << value << std::endl;
             */

    for (RotaryMap::iterator it = m_rotaries.begin() ; it != m_rotaries.end(); ++it) {
        if (it->first == controller) {
            it->second.first->setPosition(float(value));
            return ;
        }
    }
}

void
MIDIInstrumentParameterPanel::populateBankList()
{
    if (m_selectedInstrument == 0) return;

    m_bankValue->clear();
    m_banks.clear();

    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (m_selectedInstrument->getDevice());
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::populateBankList:"
        << " No MidiDevice for Instrument "
        << m_selectedInstrument->getId() << endl;
        return ;
    }

    int currentBank = -1;
    BankList banks;

    /*
    RG_DEBUG << "MIDIInstrumentParameterPanel::populateBankList: "
             << "variation type is " << md->getVariationType() << endl;
             */

    if (md->getVariationType() == MidiDevice::NoVariations) {

        banks = md->getBanks(m_selectedInstrument->isPercussion());

        if (!banks.empty()) {
            if (m_bankLabel->isHidden()) {
                m_bankLabel->show();
                m_bankCheckBox->show();
                m_bankValue->show();
            }
        } else {
            m_bankLabel->hide();
            m_bankCheckBox->hide();
            m_bankValue->hide();
        }

        for (unsigned int i = 0; i < banks.size(); ++i) {
            if (m_selectedInstrument->getProgram().getBank() == banks[i]) {
                currentBank = i;
            }
        }

    } else {

        MidiByteList bytes;
        bool useMSB = (md->getVariationType() == MidiDevice::VariationFromLSB);

        if (useMSB) {
            bytes = md->getDistinctMSBs(m_selectedInstrument->isPercussion());
        } else {
            bytes = md->getDistinctLSBs(m_selectedInstrument->isPercussion());
        }

        if (bytes.size() < 2) {
            if (!m_bankLabel->isHidden()) {
                m_bankLabel->hide();
                m_bankCheckBox->hide();
                m_bankValue->hide();
            }
        } else {
            if (m_bankLabel->isHidden()) {
                m_bankLabel->show();
                m_bankCheckBox->show();
                m_bankValue->show();
            }
        }

        if (useMSB) {
            for (unsigned int i = 0; i < bytes.size(); ++i) {
                BankList bl = md->getBanksByMSB
                              (m_selectedInstrument->isPercussion(), bytes[i]);
                RG_DEBUG << "MIDIInstrumentParameterPanel::populateBankList: have " << bl.size() << " variations for msb " << bytes[i] << endl;

                if (bl.size() == 0)
                    continue;
                if (m_selectedInstrument->getMSB() == bytes[i]) {
                    currentBank = banks.size();
                }
                banks.push_back(bl[0]);
            }
        } else {
            for (unsigned int i = 0; i < bytes.size(); ++i) {
                BankList bl = md->getBanksByLSB
                              (m_selectedInstrument->isPercussion(), bytes[i]);
                RG_DEBUG << "MIDIInstrumentParameterPanel::populateBankList: have " << bl.size() << " variations for lsb " << bytes[i] << endl;
                if (bl.size() == 0)
                    continue;
                if (m_selectedInstrument->getLSB() == bytes[i]) {
                    currentBank = banks.size();
                }
                banks.push_back(bl[0]);
            }
        }
    }

    for (BankList::const_iterator i = banks.begin();
            i != banks.end(); ++i) {
        m_banks.push_back(*i);
        m_bankValue->addItem(QObject::tr(i->getName().c_str()));
    }

    // Keep bank value enabled if percussion map is in use
    if  (m_percussionCheckBox->isChecked()) {
        m_bankValue->setDisabled(false);
    } else {
        m_bankValue->setEnabled(m_selectedInstrument->sendsBankSelect());
    }    

    if (currentBank < 0 && !banks.empty()) {
        m_bankValue->setCurrentIndex(0);
        slotSelectBank(0);
    } else {
        m_bankValue->setCurrentIndex(currentBank);
    }
}

void
MIDIInstrumentParameterPanel::populateProgramList()
{
    if (m_selectedInstrument == 0)
        return ;

    m_programValue->clear();
    m_programs.clear();

    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (m_selectedInstrument->getDevice());
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::populateProgramList: No MidiDevice for Instrument "
        << m_selectedInstrument->getId() << endl;
        return ;
    }

    /*
    RG_DEBUG << "MIDIInstrumentParameterPanel::populateProgramList:"
             << " variation type is " << md->getVariationType() << endl;
    */

    MidiBank bank( m_selectedInstrument->isPercussion(),
                   m_selectedInstrument->getMSB(),
                   m_selectedInstrument->getLSB());

    if (m_selectedInstrument->sendsBankSelect()) {
        bank = m_selectedInstrument->getProgram().getBank();
    }

    int currentProgram = -1;

    ProgramList programs = md->getPrograms(bank);

    if (!programs.empty()) {
        if (m_programLabel->isHidden()) {
            m_programLabel->show();
            m_programCheckBox->show();
            m_programValue->show();
            m_evalMidiPrgChgCheckBox->show();
            m_evalMidiPrgChgLabel->show();
        }
    } else {
        m_programLabel->hide();
        m_programCheckBox->hide();
        m_programValue->hide();
        m_evalMidiPrgChgCheckBox->hide();
        m_evalMidiPrgChgLabel->hide();
    }

    for (unsigned int i = 0; i < programs.size(); ++i) {
        std::string programName = programs[i].getName();
        if (programName != "") {
            m_programValue->addItem(QObject::tr("%1. %2")
                                       .arg(programs[i].getProgram() + 1)
                                       .arg(QObject::tr(programName.c_str())));
            if (m_selectedInstrument->getProgram() == programs[i]) {
                currentProgram = m_programs.size();
            }
            m_programs.push_back(programs[i]);
        }
    }

    // Keep program value enabled if percussion map is in use
    if  (m_percussionCheckBox->isChecked()) {
        m_programValue->setDisabled(false);
    } else {
        m_programValue->setEnabled(m_selectedInstrument->sendsProgramChange());
    }    

    if (currentProgram < 0 && !m_programs.empty()) {
        m_programValue->setCurrentIndex(0);
        slotSelectProgram(0);
    } else {
        m_programValue->setCurrentIndex(currentProgram);

        // Ensure that stored program change value is same as the one
        // we're now showing (BUG 937371)
        //
        if (!m_programs.empty()) {
            m_selectedInstrument->setProgramChange
            ((m_programs[m_programValue->currentIndex()]).getProgram());
        }
    }
}

void
MIDIInstrumentParameterPanel::populateVariationList()
{
    if (m_selectedInstrument == 0)
        return ;

    m_variationValue->clear();
    m_variations.clear();

    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (m_selectedInstrument->getDevice());
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::populateVariationList: No MidiDevice for Instrument "
        << m_selectedInstrument->getId() << endl;
        return ;
    }

    /*
    RG_DEBUG << "MIDIInstrumentParameterPanel::populateVariationList:"
             << " variation type is " << md->getVariationType() << endl;
    */

    if (md->getVariationType() == MidiDevice::NoVariations) {
        if (!m_variationLabel->isHidden()) {
            m_variationLabel->hide();
            m_variationCheckBox->hide();
            m_variationValue->hide();
        }
        return ;
    }

    bool useMSB = (md->getVariationType() == MidiDevice::VariationFromMSB);
    MidiByteList variations;

    if (useMSB) {
        MidiByte lsb = m_selectedInstrument->getLSB();
        variations = md->getDistinctMSBs(m_selectedInstrument->isPercussion(),
                                         lsb);
        RG_DEBUG << "MIDIInstrumentParameterPanel::populateVariationList: have " << variations.size() << " variations for lsb " << lsb << endl;

    } else {
        MidiByte msb = m_selectedInstrument->getMSB();
        variations = md->getDistinctLSBs(m_selectedInstrument->isPercussion(),
                                         msb);
        RG_DEBUG << "MIDIInstrumentParameterPanel::populateVariationList: have " << variations.size() << " variations for msb " << msb << endl;
    }

    m_variationValue->setCurrentIndex( -1);

    MidiProgram defaultProgram;

    if (useMSB) {
        defaultProgram = MidiProgram
                         (MidiBank(m_selectedInstrument->isPercussion(),
                                   0,
                                   m_selectedInstrument->getLSB()),
                          m_selectedInstrument->getProgramChange());
    } else {
        defaultProgram = MidiProgram
                         (MidiBank(m_selectedInstrument->isPercussion(),
                                   m_selectedInstrument->getMSB(),
                                   0),
                          m_selectedInstrument->getProgramChange());
    }
    std::string defaultProgramName = md->getProgramName(defaultProgram);

    int currentVariation = -1;

    for (unsigned int i = 0; i < variations.size(); ++i) {

        MidiProgram program;

        if (useMSB) {
            program = MidiProgram
                      (MidiBank(m_selectedInstrument->isPercussion(),
                                variations[i],
                                m_selectedInstrument->getLSB()),
                       m_selectedInstrument->getProgramChange());
        } else {
            program = MidiProgram
                      (MidiBank(m_selectedInstrument->isPercussion(),
                                m_selectedInstrument->getMSB(),
                                variations[i]),
                       m_selectedInstrument->getProgramChange());
        }

        std::string programName = md->getProgramName(program);

        if (programName != "") { // yes, that is how you know whether it exists
            /*
                    m_variationValue->addItem(programName == defaultProgramName ?
                                 tr("(default)") :
                                 strtoqstr(programName));
            */
            m_variationValue->addItem(QObject::tr("%1. %2")
                                         .arg(variations[i] + 1)
                                         .arg(QObject::tr(programName.c_str())));
            if (m_selectedInstrument->getProgram() == program) {
                currentVariation = m_variations.size();
            }
            m_variations.push_back(variations[i]);
        }
    }

    if (currentVariation < 0 && !m_variations.empty()) {
        m_variationValue->setCurrentIndex(0);
        slotSelectVariation(0);
    } else {
        m_variationValue->setCurrentIndex(currentVariation);
    }

    if (m_variations.size() < 2) {
        if (!m_variationLabel->isHidden()) {
            m_variationLabel->hide();
            m_variationCheckBox->hide();
            m_variationValue->hide();
        }

    } else {
        //!!! seem to have problems here -- the grid layout doesn't
        //like us adding stuff in the middle so if we go from 1
        //visible row (say program) to 2 (program + variation) the
        //second one overlaps the control knobs

        if (m_variationLabel->isHidden()) {
            m_variationLabel->show();
            m_variationCheckBox->show();
            m_variationValue->show();
        }

        if (m_programValue->width() > m_variationValue->width()) {
            m_variationValue->setMinimumWidth(m_programValue->width());
        } else {
            m_programValue->setMinimumWidth(m_variationValue->width());
        }
    }

    // Keep variation value enabled if percussion map is in use
    if  (m_percussionCheckBox->isChecked()) {
        m_variationValue->setDisabled(false);
    } else {
        m_variationValue->setEnabled(m_selectedInstrument->sendsBankSelect());
    }    
}

// Fill the fixed channel list controls
// @author Tom Breton (Tehom)
void
MIDIInstrumentParameterPanel::
populateChannelList(void)
{
    // Block signals
    m_channelUsed-> blockSignals(true);

    const Instrument * instrument = m_selectedInstrument;
    bool hasFixedChannel = instrument->hasFixedChannel();
    int index = hasFixedChannel ? 1 : 0;
    m_channelUsed->setCurrentIndex(index);

    // Unblock signals
    m_channelUsed-> blockSignals(false);
}

void
MIDIInstrumentParameterPanel::slotTogglePercussion(bool value)
{
    if (m_selectedInstrument == 0) {
        m_percussionCheckBox->setChecked(false);
        emit updateAllBoxes();
        return ;
    }

    m_selectedInstrument->setPercussion(value);

    populateBankList();
    populateProgramList();
    populateVariationList();

    emit changeInstrumentLabel(m_selectedInstrument->getId(),
                               m_selectedInstrument->
                                         getProgramName().c_str());
    emit updateAllBoxes();

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}

void
MIDIInstrumentParameterPanel::slotToggleBank(bool value)
{
    if (m_selectedInstrument == 0) {
        m_bankCheckBox->setChecked(false);
        emit updateAllBoxes();
        return ;
    }

    m_variationCheckBox->setChecked(value);
    m_selectedInstrument->setSendBankSelect(value);

    // Keep bank value enabled if percussion map is in use
    if  (m_percussionCheckBox->isChecked()) {
        m_bankValue->setDisabled(false);
    } else {
        m_bankValue->setDisabled(!value);
    }

    populateBankList();
    populateProgramList();
    populateVariationList();

    emit changeInstrumentLabel(m_selectedInstrument->getId(),
                               m_selectedInstrument->
                                         getProgramName().c_str());
    emit updateAllBoxes();

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}

void
MIDIInstrumentParameterPanel::slotToggleProgramChange(bool value)
{
    if (m_selectedInstrument == 0) {
        m_programCheckBox->setChecked(false);
        emit updateAllBoxes();
        return ;
    }

    m_selectedInstrument->setSendProgramChange(value);

    // Keep program value enabled if percussion map is in use
    if  (m_percussionCheckBox->isChecked()) {
        m_bankValue->setDisabled(false);
    } else {
        m_programValue->setDisabled(!value);
    }

    populateProgramList();
    populateVariationList();

    emit changeInstrumentLabel(m_selectedInstrument->getId(),
                               m_selectedInstrument->
                                         getProgramName().c_str());
    emit updateAllBoxes();

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}

void
MIDIInstrumentParameterPanel::slotToggleVariation(bool value)
{
    if (m_selectedInstrument == 0) {
        m_variationCheckBox->setChecked(false);
        emit updateAllBoxes();
        return ;
    }

    m_bankCheckBox->setChecked(value);
    m_selectedInstrument->setSendBankSelect(value);

    // Keep variation value enabled if percussion map is in use
    if  (m_percussionCheckBox->isChecked()) {
        m_bankValue->setDisabled(false);
    } else {
        m_variationValue->setDisabled(!value);
    }

    populateVariationList();

    emit changeInstrumentLabel(m_selectedInstrument->getId(),
                               m_selectedInstrument->
                                         getProgramName().c_str());
    emit updateAllBoxes();

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}

void
MIDIInstrumentParameterPanel::slotSelectBank(int index)
{
    if (m_selectedInstrument == 0)
        return ;

    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (m_selectedInstrument->getDevice());
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::slotSelectBank: No MidiDevice for Instrument "
        << m_selectedInstrument->getId() << endl;
        return ;
    }

    const MidiBank *bank = &m_banks[index];

    bool change = false;

    if (md->getVariationType() != MidiDevice::VariationFromLSB) {
        if (m_selectedInstrument->getLSB() != bank->getLSB()) {
            m_selectedInstrument->setLSB(bank->getLSB());
            change = true;
        }
    }
    if (md->getVariationType() != MidiDevice::VariationFromMSB) {
        if (m_selectedInstrument->getMSB() != bank->getMSB()) {
            m_selectedInstrument->setMSB(bank->getMSB());
            change = true;
        }
    }

    populateProgramList();

    if (change) {
        emit updateAllBoxes();
    }

    emit changeInstrumentLabel(m_selectedInstrument->getId(),
            m_selectedInstrument->getProgramName().c_str());

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}





void MIDIInstrumentParameterPanel::slotSelectProgramNoSend(int prog, int bank_lsb, int bank_msb )
{
    /*
     * This function changes the program-list entry, if
     * a midi program change message occured.
     * 
     * (the slot is being connected in RosegardenMainWindow.cpp,
     *  and called (signaled) by SequenceManger.cpp)
     * 
     * parameters:
     * prog : the program to select (triggered by program change message)
     * bank_lsb : the bank to select (if no bank-select occured, this is -1)
     *                (triggered by bank-select message (fine,lsb value))
     * bank_msb : coarse/msb value  (-1 if not specified)
     */
    if( ! this->m_evalMidiPrgChgCheckBox->isChecked() ){
        return;
    }
    
    
    
    
    if (m_selectedInstrument == 0)
        return ;

    MidiDevice *md = dynamic_cast<MidiDevice*>
            (m_selectedInstrument->getDevice());
    
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::slotSelectBank: No MidiDevice for Instrument "
                << m_selectedInstrument->getId() << endl;
        return ;
    }
    
    bool changed_bank = false;
    // bank msb value (MSB, coarse)
    if ((bank_msb >= 0) ){ // and md->getVariationType() != MidiDevice::VariationFromMSB ) {
        if (m_selectedInstrument->getMSB() != bank_msb ) {
            m_selectedInstrument->setMSB( bank_msb );
            changed_bank = true;
        }
    }    
    // selection of bank (LSB, fine)
    if ((bank_lsb >= 0) ){ //and md->getVariationType() != MidiDevice::VariationFromLSB) {
        if (m_selectedInstrument->getLSB() != bank_lsb ) {
            m_selectedInstrument->setLSB( bank_lsb );
            changed_bank = true;
        }
    }
    
    bool change = false;
    if (m_selectedInstrument->getProgramChange() != (MidiByte)prog) {
        m_selectedInstrument->setProgramChange( (MidiByte)prog );
        change = true;
    }
    
    //populateVariationList();
    
    if (change or changed_bank) {
        //emit changeInstrumentLabel( m_selectedInstrument->getId(),
        //            strtoqstr(m_selectedInstrument->getProgramName()) );
        emit updateAllBoxes();
        
        emit instrumentParametersChanged(m_selectedInstrument->getId());
    }
    
}











void
MIDIInstrumentParameterPanel::slotSelectProgram(int index)
{
    const MidiProgram *prg = &m_programs[index];
    if (prg == 0) {
        RG_DEBUG << "program change not found in bank" << endl;
        return ;
    }

    bool change = false;
    if (m_selectedInstrument->getProgramChange() != prg->getProgram()) {
        m_selectedInstrument->setProgramChange(prg->getProgram());
        change = true;
    }

    populateVariationList();

    if (change) {
        emit changeInstrumentLabel(m_selectedInstrument->getId(),
                                   m_selectedInstrument->
                                             getProgramName().c_str());
        emit updateAllBoxes();
    }

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}

void
MIDIInstrumentParameterPanel::slotSelectVariation(int index)
{
    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (m_selectedInstrument->getDevice());
    if (!md) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::slotSelectVariation: No MidiDevice for Instrument "
        << m_selectedInstrument->getId() << endl;
        return ;
    }

    if (index < 0 || index > int(m_variations.size())) {
        RG_DEBUG << "WARNING: MIDIInstrumentParameterPanel::slotSelectVariation: index " << index << " out of range" << endl;
        return ;
    }

    MidiByte v = m_variations[index];

    if (md->getVariationType() == MidiDevice::VariationFromLSB) {
        if (m_selectedInstrument->getLSB() != v) {
            m_selectedInstrument->setLSB(v);
        }
    } else if (md->getVariationType() == MidiDevice::VariationFromMSB) {
        if (m_selectedInstrument->getMSB() != v) {
            m_selectedInstrument->setMSB(v);
        }
    }

    emit instrumentParametersChanged(m_selectedInstrument->getId());
}

// In place of the old sendBankAndProgram, instruments themselves now
// signal the affected channel managers when changed.

void
MIDIInstrumentParameterPanel::slotControllerChanged(int controllerNumber)
{

    RG_DEBUG << "MIDIInstrumentParameterPanel::slotControllerChanged - "
    << "controller = " << controllerNumber << "\n";


    if (m_selectedInstrument == 0)
        return ;

    MidiDevice *md = dynamic_cast<MidiDevice*>
                     (m_selectedInstrument->getDevice());
    if (!md)
        return ;

    /*
    ControlParameter *controller = 
    md->getControlParameter(MidiByte(controllerNumber));
        */

    int value = getValueFromRotary(controllerNumber);

    if (value == -1) {
        RG_DEBUG << "MIDIInstrumentParameterPanel::slotControllerChanged - "
        << "couldn't get value of rotary for controller "
        << controllerNumber << endl;
        return ;
    }

    m_selectedInstrument->setControllerValue(MidiByte(controllerNumber),
            MidiByte(value));

    emit updateAllBoxes();
    emit instrumentParametersChanged(m_selectedInstrument->getId());

}

int
MIDIInstrumentParameterPanel::getValueFromRotary(int rotary)
{
    for (RotaryMap::iterator it = m_rotaries.begin(); it != m_rotaries.end(); ++it) {
        if (it->first == rotary)
            return int(it->second.first->getPosition());
    }

    return -1;
}

void
MIDIInstrumentParameterPanel::showAdditionalControls(bool showThem)
{
    // Now that the MIDI IPB can scroll, and since nobody seems to have
    // implemented/repaired the tab layout mode (probably by design, and a
    // good design) we'll do away with putting a limit on the number of
    // controllers visible, and just always show them all
    showThem = true;

    m_instrumentLabel->setShown(showThem);
    int index = 0;
    for (RotaryMap::iterator it = m_rotaries.begin(); it != m_rotaries.end(); ++it) {
        it->second.first->parentWidget()->setShown(showThem);
        //it->second.first->setShown(showThem || (index < 8));
        //it->second.second->setShown(showThem || (index < 8));
        index++;
    }
}

void
MIDIInstrumentParameterPanel::
slotSetUseChannel(int index)
{
    if (m_selectedInstrument == 0)
        { return; }
    if (index == 1) {
        m_selectedInstrument->setFixedChannel();
    } else {
        m_selectedInstrument->releaseFixedChannel();
    }
}

}
#include "MIDIInstrumentParameterPanel.moc"
