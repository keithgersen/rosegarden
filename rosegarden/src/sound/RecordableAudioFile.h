// -*- c-basic-offset: 4 -*-

/*
    Rosegarden
    A sequencer and musical notation editor.

    This program is Copyright 2000-2008
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

#ifndef _RECORDABLE_AUDIO_FILE_H_
#define _RECORDABLE_AUDIO_FILE_H_

#include "RingBuffer.h"
#include "AudioFile.h"

#include <vector>

namespace Rosegarden
{

// A wrapper class for writing out a recording file.  We assume the
// data is provided by a process thread and the writes are requested
// by a disk thread.
//
class RecordableAudioFile
{
public:
    typedef float sample_t;

    typedef enum
    {
        IDLE,
        RECORDING,
        DEFUNCT
    } RecordStatus;

    RecordableAudioFile(AudioFile *audioFile, // should be already open for writing
                        size_t bufferSize);
    ~RecordableAudioFile();

    void setStatus(const RecordStatus &status) { m_status = status; }
    RecordStatus getStatus() const { return m_status; }

    size_t buffer(const sample_t *data, int channel, size_t frames);
    void write();

protected:
    AudioFile            *m_audioFile;
    RecordStatus          m_status;

    std::vector<RingBuffer<sample_t> *> m_ringBuffers; // one per channel
};

}

#endif
