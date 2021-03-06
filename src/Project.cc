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
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Project.h>

void Project::set_parent ( Project *parent )
{
    assert ( this->parent == nullptr );
    assert ( parent != nullptr );
    this->parent = parent;
}


bool Project::is_root () const
{
    return this->parent == nullptr;
}

Project::Project( const char *name )
{
    assert ( name != nullptr );
    this->name = name;
}
Project::~Project()
{
    for ( auto child : child_projects ) {
        delete child;
    }
}
const std::string & Project::get_project_name () const
{
    return this->name;
}

std::string Project::get_name () const
{
    if ( parent != nullptr && !parent->is_root () ) {
        return parent->get_name () + "." + name;
    }
    return name;
}

void Project::add_subproject ( Project *child )
{
    this->child_projects.push_back ( child );
    child->set_parent ( this );
}

void Project::add_note ( Note *note )
{
    this->notes.push_back ( note );
}

void Project::print ()
{
    std::cout << "Project: " << this->get_name () << std::endl;
    std::cout << "         " << this->notes.size () << " # notes." << std::endl;
    for ( auto pr : child_projects ) {
        pr->print ();
    }
}

std::string Project::get_path ()
{
    return parent->get_path () + "/" + name;
}

std::string Project::get_relative_path ()
{
    if ( parent->is_root () ) {
        return name;
    }
    return parent->get_relative_path () + "/" + name;
}
std::string Project::get_relative_name ( const Project *p )
{
    if ( parent == p ) {
        return name;
    }
    return parent->get_relative_name ( p ) + "." + name;
}

void Project::list_projects ()
{
    if ( !this->is_root () ) {
        std::cout << this->get_name () << std::endl;
    }
    for ( auto pr : child_projects ) {
        pr->list_projects ();
    }
}

/**
 * Helper to check if directory exsits.
 */
static bool directory_exists ( const std::string &directory )
{
    if ( !directory.empty () ) {
        if ( access ( directory.c_str (), 0 ) == 0 ) {
            struct stat status;
            stat ( directory.c_str (), &status );
            if ( status.st_mode & S_IFDIR ) {
                return true;
            }
        }
    }
    // if any condition fails
    return false;
}

bool Project::check_and_create_path ()
{
    if ( !this->is_root () ) {
        if ( !parent->check_and_create_path () ) {
            return false;
        }
    }
    auto path = this->get_path ();
    if ( !directory_exists ( path ) ) {
        if ( mkdir ( path.c_str (), 0700 ) != 0 ) {
            fprintf ( stderr, "Failed to create project: %s:%s\n",
                      this->get_name ().c_str (),
                      strerror ( errno ) );
            return false;
        }
    }
    return true;
}
void Project::remove_note ( Note *note )
{
    this->notes.remove ( note );
}
