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
#ifndef __NOTE_H__
#define __NOTE_H__
#define _XOPEN_SOURCE    700
#include <time.h>
#include <list>

// Forward declaration.
class Project;
class Settings;

/**
 * Class representing a note.
 *
 * TODO how to get a consistent ID?
 * Sort list by ... then assign?
 *
 * TODO: Calculate a CRC off the content of the note?
 */
class Note
{
private:
    Project     *project  = nullptr;
    Settings    *settings = nullptr;
    std::string filename;

// Note properties
    std::string   title          = "unset";
    struct tm     last_edit_time = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned int  id             = 0;
    unsigned long revision       = 0;

    uint32_t      hash = 0;

public:
    Note( Project *project, Settings *settings, const char *filename );
    Note ( Project *p, Settings *settings );

    void print ();

    uint32_t get_body_crc ()
    {
        return this->hash;
    }

    void set_id ( unsigned int id );

    time_t get_time_t ();

    const std::string &get_title () const
    {
        return this->title;
    }


    unsigned int get_id () const
    {
        return this->id;
    }

    const std::string get_project_name ()
    {
        return this->project->get_name ();
    }
    const Project * get_project ()
    {
        return this->project;
    }

    const std::string get_relative_project_name ( const Project *p )
    {
        return this->project->get_relative_name ( p );
    }

    std::string get_modtime ();

    unsigned long get_revision () const;

    std::string get_relative_path ()
    {
        if ( this->project->is_root () ) {
            return this->filename;
        }
        return this->project->get_relative_path () + "/" + this->filename;
    }

    /**
     * Compile the note and view it.
     */
    void view ();

    /**
     * Edit the note.
     */
    bool edit ();

    /**
     * Cat the node to stdout.
     */
    bool cat ();

    // Delete a note.
    bool del ();

    bool move ( Project *p );

    bool export_to_file_html ( const std::string file );
    bool export_to_file_raw ( const std::string file );

    bool import ( const std::string path );
private:
    bool write_body ( FILE *fpout );
    void write_header ( FILE *header );
    bool generate_html ( std::string output_path );
    /**
     * This function does nothing more then a copy
     * till end of file.
     */
    void copy_till_end_of_file ( FILE *fp_edited_in, FILE *fpout );
    unsigned int calculate_crc ( FILE *fp );
    void read_title ( FILE *fp );
};

#endif
