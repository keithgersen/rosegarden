/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.

    This program is Copyright 2000-2007
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <richard.bown@ferventsoftware.com>

    The moral rights of Guillaume Laurent, Chris Cannam, and Richard
    Bown to claim authorship of this work have been asserted.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_GUITARCHORDEDITOR2_H_
#define _RG_GUITARCHORDEDITOR2_H_

#include <kdialogbase.h>

#include "Chord2.h"
#include "ChordMap2.h"

class KLineEdit;
class QComboBox;
class QSpinBox;

namespace Rosegarden
{

class FingeringBox2;


class GuitarChordEditorDialog : public KDialogBase
{
    Q_OBJECT
    
public:
	GuitarChordEditorDialog(Chord2&, const ChordMap2& chordMap, QWidget *parent=0);

protected slots:
    void slotStartFretChanged(int);
    virtual void slotOk();
    
protected:

    void populateExtensions(const QStringList&);

    FingeringBox2* m_fingeringBox;
    QComboBox* m_rootNotesList;
    QSpinBox* m_startFret;
    KLineEdit* m_ext;
    Chord2& m_chord;
    const ChordMap2& m_chordMap;    
};

}

#endif /*_RG_GUITARCHORDEDITOR2_H_*/
