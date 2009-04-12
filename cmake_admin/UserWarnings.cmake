# Rosegarden
# A MIDI and audio sequencer and musical notation editor.
#
# This program is Copyright 2000-2008
#     Guillaume Laurent   <glaurent@telegraph-road.org>,
#     Chris Cannam        <cannam@all-day-breakfast.com>,
#     Richard Bown        <richard.bown@ferventsoftware.com>
#
# The moral rights of Guillaume Laurent, Chris Cannam, and Richard
# Bown to claim authorship of this work have been asserted.
#
# This file is Copyright 2006-2008
#     Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
#
# Other copyrights also apply to some parts of this work.  Please
# see the AUTHORS file and individual file headers for details.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.  See the file
# COPYING included with this distribution for more information.

MESSAGE("\n"
"Installation Summary\n"
"--------------------\n"
"\n"
"Install Directory             : ${CMAKE_INSTALL_PREFIX}\n"
"Build type                    : ${CMAKE_BUILD_TYPE}\n"
"Use Qt/KDE precompiled headers: ${USE_PCH}\n"
"\n"
"Xft notation font support     : ${HAVE_XFT}")

MESSAGE("LIRC infrared remote support  : ${HAVE_LIRC}")

MESSAGE("")

MESSAGE(
"ALSA MIDI support             : ${HAVE_ALSA}\n"
"JACK audio support            : ${HAVE_JACK}\n"
"LADSPA plugin support         : ${HAVE_LADSPA}\n"
"DSSI synth plugin support     : ${HAVE_DSSI}\n"
"Custom OSC plugin GUI support : ${HAVE_LIBLO}\n"
"Audio timestretching          : ${HAVE_FFTW3F}\n"
"LRDF plugin metadata support  : ${HAVE_LIBLRDF}")

IF(NOT HAVE_ALSA)
MESSAGE("\n* Rosegarden requires the ALSA (Advanced Linux Sound Architecture) drivers\n"
"for MIDI, and the JACK audio framework for audio sequencing.\n"
"Please see the documentation at http://www.rosegardenmusic.com/getting/\n"
"for more information about these dependencies.")
ENDIF(NOT HAVE_ALSA)
		
IF(NOT HAVE_JACK)
MESSAGE("\n* Rosegarden uses the JACK audio server for audio recording and\n"
"sequencing.  See http://jackit.sf.net/ for more information about\n"
"getting and installing JACK.  If you want to use Rosegarden only\n"
"for MIDI, then you do not need JACK at runtime, but JACK must be available at
build time in order to compile the application.")
ENDIF(NOT HAVE_JACK)

IF(NOT HAVE_LADSPA)
MESSAGE("\n* Rosegarden supports LADSPA audio plugins if available.  See\n"
"http://www.ladspa.org/ for more information about LADSPA.  To\n"
"build Rosegarden, you need to make sure you have ladspa.h\n"
"available on your system.")
ENDIF(NOT HAVE_LADSPA)

IF(NOT HAVE_DSSI)
MESSAGE("\n* Rosegarden supports DSSI audio plugins if available.  See\n"
"http://dssi.sf.net/ for more information about DSSI.  To\n"
"build Rosegarden, you need to make sure you have dssi.h\n"
"available on your system.")
ENDIF(NOT HAVE_DSSI)

IF(NOT HAVE_LIBLO)
MESSAGE("\n* Rosegarden supports custom GUIs for DSSI (and LADSPA) plugins using\n"
"the Open Sound Control protocol.  Go to http://www.plugin.org.uk/liblo/\n"
"to obtain liblo and http://dssi.sf.net/ for more information about DSSI\n"
"GUIs.")
ENDIF(NOT HAVE_LIBLO)

IF(NOT HAVE_LIBLRDF)
MESSAGE("\n* Rosegarden requires the LRDF metadata format for classification\n"
"of LADSPA and DSSI plugins.  You can obtain LRDF from\n"
"http://www.plugin.org.uk/lrdf/.")
ENDIF(NOT HAVE_LIBLRDF)

IF(NOT HAVE_LIRC)
MESSAGE("\n* Rosegarden supports infrared remote controls, yadda bing blah\n"
"(None of us actually know anything about this, but somebody sent a patch\n"
"once, and we are no longer supporting conditional builds, so you gotta\n"
"install it to build.  Even though I don't know what it's for myself.\n"
"Them's the breaks.)")
ENDIF(NOT HAVE_LIRC)

MESSAGE("")