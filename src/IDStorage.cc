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
#include <errno.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <Colors.h>
#include <Project.h>
#include <Note.h>
#include <IDStorage.h>

void IDStorage::read ()
{
    FILE *fp = fopen ( cache_path.c_str (), "r" );
    char buffer[1024];

    if ( fp == nullptr ) {
        return;
    }

    while ( fgets ( buffer, 1024, fp ) != nullptr ) {
        char         *endpt = nullptr;
        unsigned int id     = (unsigned int) strtoul ( buffer, &endpt, 10 );
        if ( endpt != nullptr && *endpt != '\n' ) {
            endpt++;
            endpt[strlen ( endpt ) - 1] = '\0';
            idmap[endpt]                = id;
        }
    }
    fclose ( fp );
}

void IDStorage::write ()
{
    FILE *fp = fopen ( cache_path.c_str (), "w" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open id cache for writing: %s\n",
                            strerror ( errno ) );
        return;
    }
    for ( auto& x : idmap ) {
        fprintf ( fp, "%lu=%s\n", x.second, x.first.c_str () );
    }

    fclose ( fp );
}

void IDStorage::gc ( std::vector<Note *> notes )
{
    idmap.clear ();
    for ( auto note : notes ) {
        idmap[note->get_relative_path ()] = note->get_id ();
    }
    changed = true;
    notes_print_info ( "Vacuumed the id Cache.\n" );
}
unsigned int IDStorage::get_id ( const std::string path )
{
    // Not efficient, but only done once at loading.
    auto i = idmap.find ( path );
    if ( i != idmap.end () ) {
        return i->second;
    }
    // Create new id.
    // Inefficient O(n^2)?
    unsigned int id = 1;
    while ( true ) {
        bool found = false;
        for ( auto x = idmap.begin (); !found && x != idmap.end (); x++ ) {
            if ( x->second == id ) {
                found = true;
                break;
            }
        }
        if ( !found ) {
            idmap[path] = id;
            changed     = true;
            return id;
        }
        id++;
    }
}
IDStorage::IDStorage ( )
{
    std::string homedir = getenv ( "HOME" );
    if ( homedir.empty () ) {
        notes_print_error ( "Could not find home directory.\n" );
        return;
    }
    // TODO: Better way to get directory separator.
    // TODO: Use XDG_PATH to get cache directory.
    this->cache_path = homedir + "/" + ".notescc.idcache";

    read ();
}
IDStorage::~IDStorage ()
{
    if ( changed ) {
        write ();
    }
}
void IDStorage::move_id ( const std::string path_old, const std::string path_new )
{
    if ( idmap.find ( path_old ) != idmap.end () ) {
        // Get id.
        unsigned int id = idmap[path_old];
        // Delete old entry.
        idmap.erase ( path_old );
        // Add new entry.
        idmap[path_new] = id;
        changed         = true;
    }
}
void IDStorage::delete_id ( const std::string path_old )
{
    if ( idmap.find ( path_old ) != idmap.end () ) {
        idmap.erase ( path_old );
        changed = true;
    }
}
