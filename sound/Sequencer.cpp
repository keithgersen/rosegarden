// -*- c-indentation-style:"stroustrup" c-basic-offset: 4 -*-
/*
  Rosegarden-4 v0.1
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


#include "Sequencer.h"
#include <iostream>
#include "MidiArts.h"
#include "MidiFile.h"
#include "Composition.h"
#include "Event.h"
#include "NotationTypes.h"
#include "BaseProperties.h"
#include "MappedEvent.h"
#include "MappedComposition.h"

namespace Rosegarden
{

using std::cerr;
using std::cout;
using std::endl;

Sequencer::Sequencer():
    m_playStartTime(0, 0),
    m_recordStartTime(0, 0),
    m_playStartPosition(0, 0),
    m_recordStatus(ASYNCHRONOUS_MIDI),
    m_startPlayback(true),
    m_playing(false),
    m_ppq(Note(Note::Crotchet).getDuration()),
    m_tempo(120.0)     // default tempo
{
    initializeMidi();
}


Sequencer::~Sequencer()
{
}

void
Sequencer::initializeMidi()
{
    m_midiManager = Arts::Reference("global:Arts_MidiManager");
    if (m_midiManager.isNull())
    {
        cerr << "RosegardenSequencer - Can't get aRTS MidiManager" << endl;
        exit(1);
    }

    m_soundServer = Arts::Reference("global:Arts_SoundServer");
    if (m_soundServer.isNull())
    {
        cerr << "RosegardenSequencer - Can't get aRTS SoundServer" << endl;
        exit(1);
    }

    m_midiRecordPort = Arts::DynamicCast(m_soundServer.createObject("RosegardenMidiRecord"));


    if (m_midiRecordPort.isNull())
    {
        cerr << "RosegardenSequencer - Can't create aRTS MidiRecorder" << endl;
        exit(1);
    }

    m_midiPlayClient = m_midiManager.addClient(Arts::mcdPlay,
                                               Arts::mctApplication,
                                               "Rosegarden (play)","rosegarden");
    if (m_midiPlayClient.isNull())
    {
        cerr << "RosegardenSequencer - Can't create aRTS MidiClient" << endl;
        exit(1);
    }

    m_midiPlayPort = m_midiPlayClient.addOutputPort();
    if (m_midiPlayPort.isNull())
    {
        cerr << "RosegardenSequencer - Can't create aRTS Midi Output Port" << endl;
        exit(1);
    }

    m_midiRecordClient = m_midiManager.addClient(Arts::mcdRecord,
                                                 Arts::mctApplication,
                                                 "Rosegarden (record)",
                                                 "rosegarden");
    if (m_midiRecordClient.isNull())
    {
        cerr << "RosegardenSequencer - Can't create aRTS MidiRecordClient" << endl;
        exit(1);
    }

    // Create our recording midi port
    //
    m_midiRecordClient.addInputPort(m_midiRecordPort);

    // set MIDI thru - we _have_ to set this on otherwise aRTS barfs
    //
    //
    m_midiRecordPort.setMidiThru(m_midiPlayPort);

    // Set recording on
    //
    m_midiRecordPort.record(true);

}

// Check the recording status and set ourselves accordingly
// 
//
void
Sequencer::record(const RecordStatus& recordStatus)
{
    if (m_recordStatus == RECORD_MIDI ||
        m_recordStatus == RECORD_AUDIO)
        return;

    if ( recordStatus == RECORD_MIDI )
    {
        // Set status and the record start position
        //
        m_recordStatus = RECORD_MIDI;
        m_recordStartTime = m_midiRecordPort.time();
    }
    else
    if ( recordStatus == RECORD_AUDIO )
    {
        cout << "RosegardenSequencer::record() - AUDIO RECORDING not yet supported" << endl;
    }
    else
    {
        cout << "RosegardenSequencer::record() - Unsupported recording mode." << endl;
    }
  
}

Arts::TimeStamp
Sequencer::deltaTime(const Arts::TimeStamp &ts1, const Arts::TimeStamp &ts2)
{
    int usec = ts1.usec - ts2.usec;
    int sec = ts1.sec - ts2.sec;

    if ( usec < 0 )
    {
        sec--;
        usec += 1000000;
    }

    assert( sec >= 0 );

    return (Arts::TimeStamp(sec, usec));
}

Arts::TimeStamp
Sequencer::aggregateTime(const Arts::TimeStamp &ts1, const Arts::TimeStamp &ts2)
{
    unsigned int usec = ts1.usec + ts2.usec;
    unsigned int sec = ( (unsigned int) ( usec / 1000000 ) ) + ts1.sec + ts2.sec;
    usec %= 1000000;
    return(Arts::TimeStamp(sec, usec));
}


void
Sequencer::processMidiIn(const Arts::MidiCommand &midiCommand,
                         const Arts::TimeStamp &timeStamp)
{

    Rosegarden::MidiByte channel;
    Rosegarden::MidiByte message;
    //Rosegarden::Event *event;

    channel = midiCommand.status & MIDI_CHANNEL_NUM_MASK;
    message = midiCommand.status & MIDI_MESSAGE_TYPE_MASK;

    // Check for a hidden NOTE OFF (NOTE ON with zero velocity)
    if ( message == MIDI_NOTE_ON && midiCommand.data2 == 0 )
    {
        message = MIDI_NOTE_OFF;
    }

    // we use a map of Notes and this is the key
    unsigned int chanNoteKey = ( channel << 8 ) + midiCommand.data1;
    //double absoluteSecs;
    Rosegarden::RealTime duration;

    // scan for our event
    switch(message)
    {
        case MIDI_NOTE_ON:
            if ( m_noteOnMap[chanNoteKey] == 0 )
            {
                m_noteOnMap[chanNoteKey] = new MappedEvent;

                // set time since recording started in Absolute internal time
                /*
                  m_noteOnMap[chanNoteKey]->
                  setAbsoluteTime(convertToMidiTime(timeStamp));
                */

                // set note type and pitch
                //m_noteOnMap[chanNoteKey]->setType(Note::EventType);
                //m_noteOnMap[chanNoteKey]->set<Int>(BaseProperties::PITCH, midiCommand.data1);

                // Set time, pitch and velocity on the MappedEvent
                //
                m_noteOnMap[chanNoteKey]->
                       setAbsoluteTime(RealTime(timeStamp.sec, timeStamp.usec));

                m_noteOnMap[chanNoteKey]->setPitch(midiCommand.data1);
                m_noteOnMap[chanNoteKey]->setVelocity(midiCommand.data2);
    
            }
            break;

        case MIDI_NOTE_OFF:
            // if we match an open NOTE_ON
            //
            if ( m_noteOnMap[chanNoteKey] != 0 )
            {
                Rosegarden::RealTime rTS(timeStamp.sec, timeStamp.usec);
                duration = rTS - m_noteOnMap[chanNoteKey]->getAbsoluteTime();

                // for the moment, ensure we're positive like this
                //
                assert(duration.sec >= 0 || duration.usec >= 0 );

                // set the duration
                m_noteOnMap[chanNoteKey]->setDuration(duration);

                // insert the record
                    //
                    m_recordComposition.insert(m_noteOnMap[chanNoteKey]);

                    // tell us about it
#ifdef MIDI_DEBUG
                    cout << "INSERTED NOTE at time " 
                     << m_noteOnMap[chanNoteKey]->getAbsoluteTime()
                     << " of duration "
                     << m_noteOnMap[chanNoteKey]->getDuration() << endl;
#endif

                // reset the reference
                m_noteOnMap[chanNoteKey] = 0;

            }
            else
                cerr << "MIDI_NOTE_OFF with no matching MIDI_NOTE_ON" << endl;
            break;

        default:
            cout << "RosegardenSequencer - UNSUPPORTED MIDI EVENT RECEIVED" << endl;
            break;
    }
}


void
Sequencer::processMidiOut(Rosegarden::MappedComposition mappedComp,
                          const Rosegarden::RealTime &playLatency)
{
    Arts::MidiEvent event;

    // MidiCommandStatus
    //   mcsCommandMask
    //   mcsChannelMask
    //   mcsNoteOff
    //   mcsNoteOn
    //   mcsKeyPressure
    //   mcsParameter
    //   mcsProgram 
    //   mcsChannelPressure
    //   mcsPitchWheel

    // for the moment hardcode the channel
    MidiByte channel = 0;

    Rosegarden::RealTime midiRelativeTime;
    Rosegarden::RealTime midiRelativeStopTime;

    // get current port time at start of playback
    if (m_startPlayback)
    {
        m_playStartTime = m_midiPlayPort.time();
        m_startPlayback = false;
        m_playing = true;
    }


    for ( MappedComposition::iterator i = mappedComp.begin();
          i != mappedComp.end(); ++i )
    {
        // sort out the correct TimeStamp for playback
        assert((*i)->getAbsoluteTime() >= m_playStartPosition);

        // add the fiddle factor for RealTime to MIDI conversion in here
        midiRelativeTime = (*i)->getAbsoluteTime() - m_playStartPosition +
            playLatency;


        event.time = aggregateTime(m_playStartTime,
                  Arts::TimeStamp(midiRelativeTime.sec, midiRelativeTime.usec));

        midiRelativeStopTime = midiRelativeTime + (*i)->getDuration();
    
        // load the command structure
        event.command.status = Arts::mcsNoteOn | channel;
        event.command.data1 = (*i)->getPitch();     // pitch
        event.command.data2 = (*i)->getVelocity();  // velocity

        // Here's a little check to test out timing - if
        // the event TimeStamp is later than current time
        // then we've missed an event - so flag it
        //
        Arts::TimeStamp now = m_midiPlayPort.time();

        int secAhead = event.time.sec - now.sec;
        int uSecAhead = event.time.usec - now.usec;

        // Adjust for this
        if (uSecAhead < 0) 
        {
            uSecAhead += 1000000;
            secAhead++;
        }

        if (secAhead < 0)
        {
            std::cerr <<"Sequencer::processMidiOut: MIDI processing lagging by " << -secAhead << "s and " << uSecAhead << "ms" << endl;
        }

        // Send the event out to the MIDI port
        //
        m_midiPlayPort.processEvent(event);

#if 0
        int secFromStart = event.time.sec - m_playStartTime.sec;
        int usecFromStart = event.time.usec - m_playStartTime.usec;

        if (usecFromStart < 0)
        {
            secFromStart--;
            usecFromStart += 1000000;
        }


        cout << "Event sent to aRts at " << secFromStart << "s & "
             << usecFromStart << "ms" << endl;
#endif

        // and log it on the Note OFF stack
        NoteOffEvent *noteOffEvent =
            new NoteOffEvent(midiRelativeStopTime,
                     (Rosegarden::MidiByte)event.command.data1,
                     (Rosegarden::MidiByte)event.command.status);

        m_noteOffQueue.insert(noteOffEvent);

    }

    // If there's no midiRelativeTime set then set one - occurs if we're
    // not processing any NOTE ONs.
    //
    //
    if (midiRelativeTime.sec == 0 && midiRelativeTime.usec == 0)
    {
        Arts::TimeStamp now = m_midiPlayPort.time();
        int sec = now.sec - m_playStartTime.sec;
        int usec = now.usec - m_playStartTime.usec;
 
        if (usec < 0)
        {
            sec--;
            usec += 1000000;
        }

        Arts::TimeStamp relativeNow(sec, usec);
        midiRelativeTime =
               Rosegarden::RealTime(relativeNow.sec, relativeNow.usec);
    }

    // Process NOTE OFFs for current time
    processNotesOff(midiRelativeTime);
}


void
Sequencer::processNotesOff(const Rosegarden::RealTime &midiTime)
{
    Arts::MidiEvent event;
    MidiByte channel = 0;
    Rosegarden::RealTime noteOff;

    // process any pending NOTE OFFs
    for ( NoteOffQueue::iterator i = m_noteOffQueue.begin();
          i != m_noteOffQueue.end(); ++i )

    {
        // If there's a pregnant NOTE OFF around then send it
        if ((*i)->getRealTime() <= midiTime)
        {
            //event.time = m_midiPlayPort.time();

            // Conversion from RealTime to Arts::TimeStamp
            noteOff = (*i)->getRealTime();
            event.time = aggregateTime(m_playStartTime,
                           Arts::TimeStamp(noteOff.sec, noteOff.usec));

            event.command.data1 = (*i)->getPitch();
            event.command.data2 = 127;
            event.command.status = Arts::mcsNoteOff | channel;
            m_midiPlayPort.processEvent(event);

            // Remove the note from the queue
            //
            m_noteOffQueue.erase(i);
        }
    }

}

// Force all pending note offs to stop immediately.
// (Actually aRts can't do this yet, so all we can do is
// send the note off events and wait for them to clear.
// We shouldn't send NOTE OFFs with an immediate timestamp
// as some of these notes might not yet have been played.)
//
void
Sequencer::allNotesOff()
{
    Arts::MidiEvent event;
    MidiByte channel = 0;
    Rosegarden::RealTime noteOff;

    for ( NoteOffQueue::iterator i = m_noteOffQueue.begin();
          i != m_noteOffQueue.end(); ++i )
    {
        // Rather long way around of converting these two
        //
        noteOff = (*i)->getRealTime();
        event.time = aggregateTime(m_playStartTime,
                       Arts::TimeStamp(noteOff.sec, noteOff.usec));

        event.command.data1 = (*i)->getPitch();
        event.command.data2 = 127;
        event.command.status = Arts::mcsNoteOff | channel;
        m_midiPlayPort.processEvent(event);

        // Erase the event
        //
        m_noteOffQueue.erase(i);
    }
}

void 
Sequencer::initializePlayback(const Rosegarden::RealTime &position)
{
    m_startPlayback = true;
    m_playStartTime.sec = 0;
    m_playStartTime.usec = 0;
    m_playStartPosition = position;
}

void
Sequencer::stopPlayback()
{
    allNotesOff();
    m_playing = false;
}

Rosegarden::RealTime
Sequencer::getSequencerTime()
{
    if (m_playing)
    {
        Arts::TimeStamp artsTimeNow = m_midiPlayPort.time();

        Rosegarden::RealTime internalRelativeTime =
            Rosegarden::RealTime(artsTimeNow.sec, artsTimeNow.usec) -
            Rosegarden::RealTime(m_playStartTime.sec, m_playStartTime.usec);

        return (m_playStartPosition + internalRelativeTime);
    }

    return Rosegarden::RealTime(0, 0);
}

// Returns a Composition containing the latest recording
// Events or Audio
//
//
MappedComposition
Sequencer::getMappedComposition()
{
    // clear out our global segment
    //
    m_recordComposition.clear();

    // Return empty set if we're currently not recording
    //
    if (m_recordStatus != RECORD_MIDI &&
        m_recordStatus != RECORD_AUDIO)
        return m_recordComposition;

    vector<Arts::MidiEvent>::iterator midiQueueIt;
    vector<Arts::MidiEvent> *midiQueue;

    // get the MIDI queue
    //
    midiQueue = m_midiRecordPort.getQueue();

    // Process it into the internal Composition
    for (midiQueueIt = midiQueue->begin();
         midiQueueIt != midiQueue->end();
         midiQueueIt++)
    {
        processMidiIn(midiQueueIt->command,
                      recordTime(midiQueueIt->time));
    }

    // Free the returned vector from here as the aRTS stub
    // has already gone ahead and allocated a new one for
    // next time - nice leak, teach me to blindly copy
    // code .. [rwb]
    //
    delete midiQueue;

    return m_recordComposition;

}



}


