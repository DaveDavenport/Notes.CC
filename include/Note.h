#ifndef __NOTE_H__
#define __NOTE_H__
#define _XOPEN_SOURCE    700
#include <time.h>

// Forward declaration.
class Project;

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
    Project     *project = nullptr;
    std::string filename;

// Note properties
    std::string  title;
    std::string  last_edit;
    struct tm    last_edit_time;
    unsigned int id = 0;

public:
    Note( Project *project, const char *filename );

    void print ();

// TODO
    unsigned int calculate_body_crc ()
    {
        return 0;
    }

    void set_id ( unsigned int id );
    time_t get_time_t ();
    std::string get_title () const
    {
        return this->title;
    }


    unsigned int get_id() const
    {
        return this->id;
    }

    std::string get_project()
    {
        return this->project->get_name();
    }

    std::string get_modtime();
};

#endif
