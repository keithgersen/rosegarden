// -*- c-basic-offset: 4 -*-
/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2002
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "studiocommands.h"
#include "rosegardenguidoc.h"
#include "rosedebug.h"
#include "rosestrings.h"

#include "Composition.h"
#include "Studio.h"
#include "MidiDevice.h"

#include <dcopclient.h>
#include <kapp.h>
#include <qtextstream.h>


ModifyDeviceCommand::ModifyDeviceCommand(
        Rosegarden::Studio *studio,
        int device,
        const std::string &name,
        const std::string &librarianName,
        const std::string &librarianEmail,
        std::vector<Rosegarden::MidiBank> bankList,
        std::vector<Rosegarden::MidiProgram> programList,
        bool overwrite):
    KNamedCommand(getGlobalName()),
    m_studio(studio),
    m_device(device),
    m_name(name),
    m_librarianName(librarianName),
    m_librarianEmail(librarianEmail),
    m_bankList(bankList),
    m_programList(programList),
    m_overwrite(overwrite)
{
}

void
ModifyDeviceCommand::execute()
{
    Rosegarden::MidiDevice *device = m_studio->getMidiDevice(m_device);

    m_oldName = device->getName();
    m_oldBankList = device->getBanks();
    m_oldProgramList = device->getPrograms();
    m_oldLibrarianName = device->getLibrarianName();
    m_oldLibrarianEmail = device->getLibrarianEmail();

    if (m_overwrite)
    {
        device->replaceBankList(m_bankList);
        device->replaceProgramList(m_programList);
        device->setName(m_name);
        device->setLibrarian(m_librarianName, m_librarianEmail);
    }
    else
    {
        device->mergeBankList(m_bankList);
        device->mergeProgramList(m_programList);

        std::string mergeName = device->getName() + std::string("/") + m_name;
        device->setName(mergeName);
    }

}

void
ModifyDeviceCommand::unexecute()
{
    Rosegarden::MidiDevice *device = m_studio->getMidiDevice(m_device);

    device->setName(m_oldName);
    device->replaceBankList(m_oldBankList);
    device->replaceProgramList(m_oldProgramList);
    device->setLibrarian(m_oldLibrarianName, m_oldLibrarianEmail);

}

// -------------------- ModifyDeviceMapping -----------------------
//

ModifyDeviceMappingCommand::ModifyDeviceMappingCommand(
        RosegardenGUIDoc *doc,
        Rosegarden::DeviceId fromDevice,
        Rosegarden::DeviceId toDevice):
            KNamedCommand(getGlobalName()),
            m_composition(&doc->getComposition()),
            m_studio(&doc->getStudio()),
            m_fromDevice(fromDevice),
            m_toDevice(toDevice)
{
}

void
ModifyDeviceMappingCommand::execute()
{
    Rosegarden::Composition::trackcontainer &tracks =
        m_composition->getTracks();
    Rosegarden::Composition::trackcontainer::iterator it = tracks.begin();
    Rosegarden::Instrument *instr = 0;
    int index = 0;

    for(; it != tracks.end(); it++)
    {
        instr = m_studio->getInstrumentById(it->second->getInstrument());

        if (instr->getDevice()->getId() == m_fromDevice)
        {
            // if source and target are MIDI
            if (m_studio->getDevice(m_fromDevice)->getType() ==
                    Rosegarden::Device::Midi &&
                m_studio->getDevice(m_toDevice)->getType() ==
                    Rosegarden::Device::Midi)
            {
                // Try to match channels on the target device
                //
                Rosegarden::MidiByte channel = instr->getMidiChannel();

                Rosegarden::InstrumentList destList = m_studio->
                    getDevice(m_toDevice)->getPresentationInstruments();

                Rosegarden::InstrumentList::iterator dIt = destList.begin();

                for (; dIt != destList.end(); dIt++)
                {
                    if ((*dIt)->getMidiChannel() == channel)
                    {
                        break;
                    }
                }

                // Failure to match anything and there's no Instruments
                // at all in the destination.  Skip to next source Instrument.
                //
                if (dIt == destList.end() || destList.size() == 0)
                    continue;


                RG_DEBUG << " Track " << it->first  
                         << ", setting Instrument to "
                         << (*dIt)->getId() << endl;

                // store "to" and "from" values
                //
                m_mapping.push_back(
                        std::pair<Rosegarden::TrackId,
                                  Rosegarden::InstrumentId>
                                  (it->first,
                                   instr->getId()));

                it->second->setInstrument((*dIt)->getId());
            }
            else // audio is involved in the mapping - use indexes
            {
                // assign by index numbers
                Rosegarden::InstrumentList destList = m_studio->
                    getDevice(m_toDevice)->getPresentationInstruments();

                // skip if we can't match
                //
                if (index > (int)(destList.size() - 1))
                    continue;

                m_mapping.push_back(
                        std::pair<Rosegarden::TrackId,
                                  Rosegarden::InstrumentId>
                                  (it->first,
                                   instr->getId()));

                it->second->setInstrument(destList[index]->getId());
            }

            index++;
        }
    }

}

void
ModifyDeviceMappingCommand::unexecute()
{
    std::vector<std::pair<Rosegarden::TrackId, Rosegarden::InstrumentId> >
        ::iterator it = m_mapping.begin();
    Rosegarden::Track *track = 0;

    for (; it != m_mapping.end(); it++)
    {
        track = m_composition->getTrackById(it->first);
        track->setInstrument(it->second);
    }
}

// ----------------------- ModifyInstrumentMapping -------------------------
//

ModifyInstrumentMappingCommand::ModifyInstrumentMappingCommand(
        RosegardenGUIDoc *doc,
        Rosegarden::InstrumentId fromInstrument,
        Rosegarden::InstrumentId toInstrument):
            KNamedCommand(getGlobalName()),
            m_composition(&doc->getComposition()),
            m_studio(&doc->getStudio()),
            m_fromInstrument(fromInstrument),
            m_toInstrument(toInstrument)
{
}

void
ModifyInstrumentMappingCommand::execute()
{
    Rosegarden::Composition::trackcontainer &tracks =
        m_composition->getTracks();
    Rosegarden::Composition::trackcontainer::iterator it = tracks.begin();

    for(; it != tracks.end(); it++)
    {
        if (it->second->getInstrument() == m_fromInstrument)
        {
            m_mapping.push_back(it->first);
            it->second->setInstrument(m_toInstrument);
        }
    }

}

void
ModifyInstrumentMappingCommand::unexecute()
{
    std::vector<Rosegarden::TrackId>::iterator it = m_mapping.begin();
    Rosegarden::Track *track = 0;

    for (; it != m_mapping.end(); it++)
    {
        track = m_composition->getTrackById(*it);
        track->setInstrument(m_fromInstrument);
    }
}

       
void
RenameDeviceCommand::execute()
{
    Rosegarden::MidiDevice *device = m_studio->getMidiDevice(m_deviceId);
    if (device) {
	if (m_oldName == "") m_oldName = device->getName();
	device->setName(m_name);
    }
}

void
RenameDeviceCommand::unexecute()
{
    Rosegarden::MidiDevice *device = m_studio->getMidiDevice(m_deviceId);
    if (device) {
	device->setName(m_oldName);
    }
}

CreateOrDeleteDeviceCommand::CreateOrDeleteDeviceCommand(Rosegarden::Studio *studio,
							 Rosegarden::DeviceId id) :
    KNamedCommand(getGlobalName(true)),
    m_studio(studio),
    m_deviceId(id),
    m_deviceCreated(true)
{
    Rosegarden::Device *device = m_studio->getDevice(m_deviceId);

    if (device) {
	m_name = device->getName();
	m_type = device->getType();
	m_direction = Rosegarden::MidiDevice::Play;
	Rosegarden::MidiDevice *md =
	    dynamic_cast<Rosegarden::MidiDevice *>(device);
	if (md) m_direction = md->getDirection();
	m_connection = device->getConnection();
    } else {
	RG_DEBUG << "CreateOrDeleteDeviceCommand: No such device as " 
		 << m_deviceId << endl;
    }
}

    
void
CreateOrDeleteDeviceCommand::execute()
{
    if (!m_deviceCreated) {

	// Create

	// don't want to do this again on undo even if it fails -- only on redo
	m_deviceCreated = true;

	{
	    QByteArray data;
	    QByteArray replyData;
	    QCString replyType;
	    QDataStream arg(data, IO_WriteOnly);

	    arg << (int)m_type;
	    arg << (unsigned int)m_direction;

	    if (!kapp->dcopClient()->call(ROSEGARDEN_SEQUENCER_APP_NAME,
					  ROSEGARDEN_SEQUENCER_IFACE_NAME,
					  "addDevice(int, unsigned int)",
					  data, replyType, replyData, false) ||
		replyType != "unsigned int") { 
		SEQMAN_DEBUG << "CreateDeviceCommand::execute - "
			     << "failure in sequencer addDevice" << endl;
		return;
	    }

	    QDataStream reply(replyData, IO_ReadOnly);
	    reply >> m_deviceId;

	    if (m_deviceId == Rosegarden::Device::NO_DEVICE) {
		SEQMAN_DEBUG << "CreateDeviceCommand::execute - "
			     << "sequencer addDevice failed" << endl;
		return;
	    }

	    SEQMAN_DEBUG << "CreateDeviceCommand::execute - "
			 << " added device " << m_deviceId << endl;
	}

	{
	    QByteArray data;
	    QByteArray replyData;
	    QCString replyType;
	    QDataStream arg(data, IO_WriteOnly);

	    arg << (unsigned int)m_deviceId;
	    arg << strtoqstr(m_connection);

	    if (!kapp->dcopClient()->call(ROSEGARDEN_SEQUENCER_APP_NAME,
					  ROSEGARDEN_SEQUENCER_IFACE_NAME,
					  "setConnection(unsigned int, QString)",
					  data, replyType, replyData, false)) {
		SEQMAN_DEBUG << "CreateDeviceCommand::execute - "
			     << "failure in sequencer setConnection" << endl;
		return;
	    }

	    SEQMAN_DEBUG << "CreateDeviceCommand::execute - "
			 << " reconnected device " << m_deviceId
			 << " to " << m_connection << endl;
	}

	// Add the device to the Studio now, so that we can name it --
	// otherwise the name will be lost
	m_studio->addDevice(m_name, m_deviceId, m_type);

    } else {

	// Delete

	QByteArray data;
	QByteArray replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);

	if (m_deviceId == Rosegarden::Device::NO_DEVICE) return;

	arg << (int)m_deviceId;

	if (!kapp->dcopClient()->call(ROSEGARDEN_SEQUENCER_APP_NAME,
				      ROSEGARDEN_SEQUENCER_IFACE_NAME,
				      "removeDevice(unsigned int)",
				      data, replyType, replyData, false)) {
	    SEQMAN_DEBUG << "CreateDeviceCommand::execute - "
			 << "failure in sequencer addDevice" << endl;
	    return;
	}

	SEQMAN_DEBUG << "CreateDeviceCommand::unexecute - "
		     << " removed device " << m_deviceId << endl;

	m_studio->removeDevice(m_deviceId);

	m_deviceId = Rosegarden::Device::NO_DEVICE;
	m_deviceCreated = false;
    }
}


void
ReconnectDeviceCommand::execute()
{
    Rosegarden::Device *device = m_studio->getDevice(m_deviceId);

    if (device) {
	m_oldConnection = device->getConnection();
	
	QByteArray data;
	QByteArray replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);

	arg << (unsigned int)m_deviceId;
	arg << strtoqstr(m_newConnection);

	if (!kapp->dcopClient()->call(ROSEGARDEN_SEQUENCER_APP_NAME,
				      ROSEGARDEN_SEQUENCER_IFACE_NAME,
				      "setConnection(unsigned int, QString)",
				      data, replyType, replyData, false)) { 
	    SEQMAN_DEBUG << "ReconnectDeviceCommand::execute - "
			 << "failure in sequencer setConnection" << endl;
	    return;
	}

	SEQMAN_DEBUG << "ReconnectDeviceCommand::execute - "
		     << " reconnected device " << m_deviceId
		     << " to " << m_newConnection << endl;
    }
}

void
ReconnectDeviceCommand::unexecute()
{
    Rosegarden::Device *device = m_studio->getDevice(m_deviceId);

    if (device) {
	
	QByteArray data;
	QByteArray replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);

	arg << (unsigned int)m_deviceId;
	arg << strtoqstr(m_oldConnection);

	if (!kapp->dcopClient()->call(ROSEGARDEN_SEQUENCER_APP_NAME,
				      ROSEGARDEN_SEQUENCER_IFACE_NAME,
				      "setConnection(unsigned int, QString)",
				      data, replyType, replyData, false)) { 
	    SEQMAN_DEBUG << "ReconnectDeviceCommand::unexecute - "
			 << "failure in sequencer setConnection" << endl;
	    return;
	}

	SEQMAN_DEBUG << "ReconnectDeviceCommand::unexecute - "
		     << " reconnected device " << m_deviceId
		     << " to " << m_oldConnection << endl;
    }
}



