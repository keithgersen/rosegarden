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

#ifndef _WARNING_WIDGET_H_
#define _WARNING_WIDGET_H_

#include "WarningDialog.h"

#include <QLabel>
#include <QToolButton>
#include <QQueue>


namespace Rosegarden
{


/** Fed with data from the sequence manager, this widget displays the status of
 * various environmental runtime factors, and provides detailed information to
 * users when sub-optimal runtime conditions are detected.  This mechanism
 * replaces the tedious parade of startup warning dialogs we used to dump in the
 * user's face the first time they tried to run Rosegarden on an ordinary
 * everyday desktop-oriented distro.
 *
 * This widget is managed by RosegardenMainWindow, which receives signals
 * primarily from SequenceManager and passes them through into here, calling
 * set___Warning(true) as appropriate, and adding a new warning message to our
 * queue.
 *
 * Adding a message to the queue causes the warning button to show itself, and
 * when clicked, the queued warning messages are displayed in a tabbed dialog.
 *
 * \author D. Michael Mcintyre
 */
class WarningWidget : public QWidget
{
    Q_OBJECT
public:

    typedef enum { Midi, Audio, Timer, Other } WarningType;

    /** Constructs a WarningWidget in a default all clear state that assumes
     * MIDI, audio, and the system timer are all functioning correctly.  This
     * widget is intended to be displayed on the status bar in the main window.
     */
    WarningWidget();
    ~WarningWidget();

    void setMidiWarning(const bool status);
    void setAudioWarning(const bool status);
    void setTimerWarning(const bool status);

    /** Add a message (consisting of a text and an informative text) to the
     * queue.  These will be displayed via displayMessageQueue() when the user
     * clicks the warning icon
     */
    void queueMessage(const QString text, const QString informativeText);

    QSize sizeHint();

    /** We'll build the message queue out of these for convenience, so both the
     * text and informative text can be tossed about as one unit
     */
    typedef std::pair<QString, QString> Message;

protected:
    QLabel *m_midiIcon;
    QLabel *m_audioIcon;
    QLabel *m_timerIcon;

    QToolButton *m_warningButton;

    QString m_text;
    QString m_informativeText;

    /** The message queue itself
     */
    QQueue<Message> m_queue;

    WarningDialog *m_warningDialog;

protected slots:

    /** Display the message queue in a suitable dialog, on demand
     */
    void displayMessageQueue();
};


}

#endif