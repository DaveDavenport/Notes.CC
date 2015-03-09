/**
 * Notes.CC
 *
 * MIT/X11 License
 * Copyright (c) 2014 Qball  Cow <qball@gmpclient.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <Project.h>
#include <Note.h>

#include <Filter.h>

NotesFilter::NotesFilter( std::vector< Note *> notes )
{
    // Copy the list!
    start_notes = std::list<Note *>( notes.begin (), notes.end () );
}

void NotesFilter::filter_not_archive ( )
{
    for ( auto iter = start_notes.begin (); iter != start_notes.end (); iter++ ) {
        Note *note = *iter;

        // Skip empty elements.
        if ( note == nullptr ) {
            continue;
        }
        bool          remove = false;
        const Project *p     = note->get_project ();
        while ( !p->is_root () && !p->get_parent ()->is_root () ) {
            p = p->get_parent ();
        }
        if ( p->get_name () != "Archive" ) {
            remove = true;
        }

        if ( remove ) {
            *iter = nullptr;
        }
    }
    start_notes.remove ( nullptr );
}
void NotesFilter::filter_archive ( )
{
    for ( auto iter = start_notes.begin (); iter != start_notes.end (); iter++ ) {
        Note *note = *iter;

        // Skip empty elements.
        if ( note == nullptr ) {
            continue;
        }
        bool          remove = false;
        const Project *p     = note->get_project ();
        while ( !p->is_root () && !p->get_parent ()->is_root () ) {
            p = p->get_parent ();
        }
        if ( p->get_name () == "Archive" ) {
            remove = true;
        }

        if ( remove ) {
            *iter = nullptr;
        }
    }
    start_notes.remove ( nullptr );
}

bool NotesFilter::add_keyword_filter ( std::string &value )
{
    size_t index = value.find ( ':' );
    if ( index != std::string::npos ) {
        if ( value.substr ( 0, index ) == "project" ) {
            auto projectn = value.substr ( index + 1, std::string::npos );
            for ( auto iter = start_notes.begin (); iter != start_notes.end (); iter++ ) {
                Note *note = *iter;

                // Skip empty elements.
                if ( note == nullptr ) {
                    continue;
                }
                auto npn    = note->get_project_name ();
                bool remove = true;
                if ( npn.size () >= projectn.size () ) {
                    if ( strcasestr ( npn.c_str (), projectn.c_str () ) != nullptr ) {
                        remove = false;
                    }
                }
                if ( remove ) {
                    *iter = nullptr;
                }
            }
            start_notes.remove ( nullptr );
            return true;
        }
    }
    return false;
}
void NotesFilter::add_filter ( std::string value )
{
    // Keyword matching.
    if ( this->add_keyword_filter ( value ) ) {
        return;
    }
    const char *val_cstr = value.c_str ();
    for ( auto iter = start_notes.begin (); iter != start_notes.end (); iter++ ) {
        Note *note = *iter;

        // Skip empty elements.
        if ( note == nullptr ) {
            continue;
        }

        bool remove = true;

        if ( strcasestr ( note->get_title ().c_str (), val_cstr ) != NULL ) {
            remove = false;
        }
        else
        if ( strcasestr ( note->get_project_name ().c_str (), val_cstr ) != NULL ) {
            remove = false;
        }

        if ( remove ) {
            *iter = nullptr;
        }
    }
    // Remove empty items.
    start_notes.remove ( nullptr );
}
