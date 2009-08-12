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

#ifndef _RG_SYMBOLINSERTER_H_
#define _RG_SYMBOLINSERTER_H_

#include "base/NotationTypes.h"
#include "NotationTool.h"
#include "base/Event.h"

#include <QString>


namespace Rosegarden
{

class ViewElement;
class NotationWidget;

/**
 * This tool will insert symbols on mouse click events
 */
class SymbolInserter : public NotationTool
{
    Q_OBJECT
    
    friend class NotationToolBox;

public:
    virtual void ready();

    virtual void handleLeftButtonPress(const NotationMouseEvent *);

    static const QString ToolName;

public slots:
    void slotSetSymbol(Symbol symbolType);

protected slots:
    void slotNotesSelected();
    void slotEraseSelected();
    void slotSelectSelected();

protected:
    SymbolInserter(NotationWidget *);
    Symbol m_symbol;
};

}

#endif