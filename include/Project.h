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
#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <list>

// Forward
class Note;

/**
 * This class represents a Project.
 *
 * A project on it own can contain sub-projects and notes.
 */
class Project
{
private:
// Pointer to the parent Project.
    Project           *parent = nullptr;
// Name of this project.
    std::string       name;
// List of notes contained in this project. (project does not own note)
    std::list<Note *> notes;

protected:
// List of children projects (projects owns it and should free it)
    std::list<Project*> child_projects;
    void set_parent ( Project *parent );

public:
    Project *get_parent () const
    {
        return this->parent;
    }
    bool is_root () const;
    Project( const char *name );

    virtual ~Project();

    std::string get_name () const;
    const std::string & get_project_name () const;

/**
 * Add hierarchical structure.
 */
    void add_subproject ( Project *child );

    void add_note ( Note *note );
    void remove_note ( Note *note );

    void print ();

    virtual std::string get_path ();
    virtual std::string get_relative_path ();
    virtual std::string get_relative_name ( const Project *p );

    void list_projects ();

    unsigned int get_num_notes () const
    {
        return this->notes.size ();
    }
    unsigned int get_num_notes_recursive ()
    {
        unsigned int value = this->notes.size ();
        for ( auto p : child_projects ) {
            value += p->get_num_notes_recursive ();
        }
        return value;
    }

    const std::list < Project *> &get_child_projects () const
    {
        return this->child_projects;
    }

    Project * find_child ( std::string &name ) const
    {
        for ( auto p : child_projects ) {
            if ( p->get_project_name () == name ) {
                return p;
            }
        }
        return nullptr;
    }

    /**
     * Recurse through the projects and creates the path in the notes repository.
     *
     * @returns false when it failed to create the path.
     */
    bool check_and_create_path ();
};

#endif
