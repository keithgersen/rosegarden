/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#ifndef DEVICESMANAGERNEW_H
#define DEVICESMANAGERNEW_H

#include "gui/ui/DevicesManagerNewUi.h"

#include "base/Device.h"
#include "base/MidiDevice.h"
#include "base/MidiTypes.h"
#include "base/Studio.h"

#include <QWidget>
#include <QDialog>
#include <QObject>


namespace Rosegarden
{


typedef std::vector<MidiDevice *> MidiDeviceList;

class RosegardenDocument;
class Studio;

/** Creates a device manager dialog
 *
 * \author Emanuel Rumpf
 */
class DevicesManagerNew : public QMainWindow, public Ui::DevicesManagerNewUi
{
    Q_OBJECT
    
public:
    
    DevicesManagerNew ( QWidget* parent, RosegardenDocument* doc );
    ~DevicesManagerNew();
    
    /**
    *    Clear all lists
    */
    void clearAllPortsLists( );
    
    /**
    *    make Slot connections
    */
    void connectSignalsToSlots( );
    
    MidiDevice* getDeviceByName( QString deviceName );
    MidiDevice* getDeviceById( DeviceId devId );
    
    MidiDevice* getMidiDeviceOfItem( QTreeWidgetItem* twItem );
    MidiDevice* getCurrentlySelectedDevice( QTreeWidget* treeWid );
    
    void connectMidiDeviceToPort ( MidiDevice* mdev, QString portName );
    
    /**
    *    If the selected device has changed, this
    *    marks (checks) the associated list entry in the ports list (connection)
    */
    void updateCheckStatesOfPortsList( QTreeWidget* treeWid_ports, QTreeWidget* treeWid_devices );
    
    /**
    *    adds/removes list entries in the visible devices-list (treeWid),
    *    if the (invisible) device-list of the sequencer has changed
    */
    void updateDevicesList( DeviceList* devices, QTreeWidget* treeWid, 
                            MidiDevice::DeviceDirection in_out_direction );
    
    /**
    *    search treeWid for the item associated with devId
    */
    QTreeWidgetItem* searchItemWithDeviceId( QTreeWidget* treeWid, DeviceId devId );
    
    QTreeWidgetItem* searchItemWithPort( QTreeWidget* treeWid, QString portName );
    
    /**
    *    add/remove list entries in the visible ports-list (connections),
    *    if the (invisible) connections of the sequencer/studio have changed.
    */
    void updatePortsList( QTreeWidget* treeWid, MidiDevice::DeviceDirection PlayRecDir );
    
    
signals:
    //void deviceNamesChanged();
    
    void editBanks ( DeviceId );
    void editControllers ( DeviceId );
    
    void sigDeviceNameChanged( DeviceId );
    
    
public slots:
    void slotOutputPortClicked( QTreeWidgetItem * item, int column );
    void slotPlaybackDeviceSelected();
    
    void slotInputPortClicked( QTreeWidgetItem * item, int column );
    void slotRecordDeviceSelected();
    void slotRecordDevicesListItemClicked( QTreeWidgetItem* item, int col);
    
    void slotDeviceItemChanged ( QTreeWidgetItem * item, int column );
    
    void slotRefreshOutputPorts();
    void slotRefreshInputPorts();
    
    void slotAddPlaybackDevice();
    void slotAddRecordDevice();
    
    void slotDeletePlaybackDevice();
    void slotDeleteRecordDevice();
    
    void slotManageBanksOfPlaybackDevice();
    void slotEditControllerDefinitions();
    
    void show();
    void slotClose();
    void slotHelpRequested();
    
protected:
    //
    RosegardenDocument *m_doc;
    Studio *m_studio;
    
    /**
    *    used to store the device ID in the QTreeWidgetItem
    *    of the visible device list (QTreeWidget)
    */
    int m_UserRole_DeviceId; // = Qt::UserRole + 1;
    
    QString m_noPortName;
};


} // end namespace Rosegarden

#endif // DEVICESMANAGERNEW_H
