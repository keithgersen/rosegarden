// -*- c-basic-offset: 4 -*-


/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2003
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    This file is Copyright 2003
        Mark Hymers         <markh@linuxfromscratch.org>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include <utility>
#include <map>
#include <string>
#include "Colour.h"

#ifndef _BASE_COLOURMAP_H_
#define _BASE_COLOURMAP_H_

// These are the default, default colour
#define COLOUR_DEF_R 197
#define COLOUR_DEF_B 125 
#define COLOUR_DEF_G 211

namespace Rosegarden 
{
    typedef std::map<unsigned int, std::pair<Colour, std::string>, std::less<unsigned int> > RCMap;

/**
 * ColourMap is our table which maps the unsigned integer keys stored in
 *  segments to both a Colour and a String containing the 'name'
 */

class ColourMap
{
public:
    // Functions:

    /**
     * Initialises an ColourMap with a default element set to
     * whatever COLOUR_DEF_X defines the colour to be (see the source file)
     */
    ColourMap();
    /**
     * Initialises an ColourMap with a default element set to
     * the value of the Colour passed in.
     */
    ColourMap(const Colour& input);
    ~ColourMap();

    /**
     * Returns the Colour associated with the item_num passed in.  Note that
     * if the item_num doesn't represent a valid item, the routine returns
     * the value of the Default colour.  This means that if somehow some of
     * the Segments get out of sync with the ColourMap and have invalid
     * colour values, they'll be set to the Composition default colour.
     */
    Colour getColourByIndex(unsigned int item_num);

    /**
     * Returns the string associated with the item_num passed in.  If the
     * item_num doesn't exist, it'll return "" (the same name as the default
     * colour has - for internationalization reasons).
     */
    std::string getNameByIndex(unsigned int item_num);

    /**
     * If item_num exists, this routine deletes it from the map.
     */
    bool deleteItemByIndex(unsigned int item_num);

    /**
     * This routine adds a Colour using the lowest possible index.
     */
    bool addItem(Colour colour, std::string name);

    /**
     * If the item with item_num exists and isn't the default, this
     * routine modifies the string associated with it
     */
    bool modifyNameByIndex(unsigned int item_num, std::string name);

    /**
     * If the item with item_num exists, this routine modifies the 
     * Colour associated with it
     */
    bool modifyColourByIndex(unsigned int item_num, Colour colour);

    /**
     * If both items exist, swap them.  
     */
    bool swapItems(unsigned int item_1, unsigned int item_2);

//    void replace(ColourMap &input);

    /**
     * This returns a const iterator pointing to m_map.begin()
     */
    RCMap::const_iterator begin();

    /**
     * This returns a const iterator pointing to m_map.end()
     */
    RCMap::const_iterator end();

    ColourMap& operator=(const ColourMap& input);

    int size() const;

private:
    RCMap m_map;
};

}

#endif
