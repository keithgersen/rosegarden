#!/usr/bin/perl -w

use strict;

print qq{
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

/*
 * This file has been automatically generated and should not be compiled.
 *
 * The purpose of the file is to collect the translation strings from the
 * presets.xml files which are used in setting the player ability.
 * 
 * The code is prepared for  lupdate  program. There is make ts target :
 *
 *   make -f qt4-makefile ts
 *
 * which is used to extract translation strings and to update ts/*.tr files.
 */

};

my $file = $ARGV[0];
my $nextfile = $ARGV[1];
my $category_name = "";
my $instrument_name = "";
while (<>) {
    if ($ARGV[0]) {
        if ($nextfile ne $ARGV[0]) {
            $file = $nextfile;
            $nextfile = $ARGV[0];
        }
    }
    my $line = $_;


    if ($line =~ /category name="([^"]*)"/) {
        $category_name = $1;
        $instrument_name = "";

        print 'QObject::tr("' . $category_name . '");';
        print ' /* ' . $file;
        print ' */
';
    } elsif ($line =~ /instrument name="([^"]*)"/) {
	$instrument_name = $1;

        print 'QObject::tr("' . $instrument_name . '");';
        print ' /* ' . $file;
        print ' : ' . $category_name;
        print ' */
';
    }
}