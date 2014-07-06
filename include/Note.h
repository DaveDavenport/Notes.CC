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
    std::string   title          = "unset";
    struct tm     last_edit_time = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned int  id             = 0;
    unsigned long revision       = 0;

    uint32_t      hash = 0;

public:
    Note( Project *project, const char *filename );
    Note ( Project *p );

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

    std::string get_modtime ();

    unsigned long get_revision () const;

    /**
     * Compile the note and view it.
     */
    void view ();

    /**
     * Edit the note.
     */
    bool edit ();

    std::string get_relative_path ()
    {
        if ( this->project->is_root () ) {
            return this->filename;
        }
        return this->project->get_relative_path () + "/" + this->filename;
    }
private:
    void write_body ( FILE *fpout );
    void write_header ( FILE *header );
    /**
     * This function does nothing more then a copy
     * till end of file.
     */
    void copy_till_end_of_file ( FILE *fp_edited_in, FILE *fpout );
    unsigned int calculate_crc ( FILE *fp );
    void read_title ( FILE *fp );
};

#endif
