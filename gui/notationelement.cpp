// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4 v0.1
    A sequencer and musical notation editor.

    This program is Copyright 2000-2001
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

#include <qcanvas.h>
#include <algorithm>

#include "notationelement.h"
#include "notationproperties.h"
#include "rosedebug.h"

#include "NotationTypes.h"
#include "Event.h"

#include <kmessagebox.h>

using Rosegarden::Event;
using Rosegarden::Note;
using Rosegarden::Int;
using Rosegarden::Bool;
using Rosegarden::timeT;

NotationElement::NotationElement(Event *event)
    : ViewElement(event),
      m_x(0),
      m_y(0),
      m_canvasItem(0)
{
}

NotationElement::~NotationElement()
{
//     kdDebug(KDEBUG_AREA) << "~NotationElement()\n";

    delete m_canvasItem;
}

double
NotationElement::getEffectiveX() throw (NoCanvasItem)
{
    if (m_canvasItem)
        return m_canvasItem->x();
    else
        throw NoCanvasItem();
}

double
NotationElement::getEffectiveY() throw (NoCanvasItem)
{
    if (m_canvasItem)
        return m_canvasItem->y();
    else
        throw NoCanvasItem();
}

void
NotationElement::setNote(Note note)
{
    event()->setDuration(note.getDuration());
    event()->setMaybe<Int>(Rosegarden::Note::NoteType, note.getNoteType());
    event()->setMaybe<Int>(Rosegarden::Note::NoteDots, note.getDots());
}

Note
NotationElement::getNote() const
{
    Note::Type ntype = event()->get<Int>(Rosegarden::Note::NoteType);
    long dots = 0;;
    event()->get<Int>(Rosegarden::Note::NoteDots, dots);
    return Note(ntype, dots);
}


bool
NotationElement::isRest() const
{
    return event()->isa("rest");
}

bool
NotationElement::isNote() const
{
    return event()->isa(Note::EventType);
}

void
NotationElement::setCanvasItem(QCanvasItem *e, double dxoffset, double dyoffset)
{
    delete m_canvasItem;
    m_canvasItem = e;
    e->move(m_x + dxoffset, m_y + dyoffset);
}

void
NotationElement::reposition(double dxoffset, double dyoffset)
{
    m_canvasItem->move(m_x + dxoffset, m_y + dyoffset);
}

#endif
