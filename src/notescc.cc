#include <iostream>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <list>

#include <Project.h>
#include <Note.h>
#include <Colors.h>
#include <TableView.h>

#include <rhash.h>

struct timespec _tick_start;
struct timespec _tick_stop;

#define INIT_TIC_TAC()    { clock_gettime ( CLOCK_REALTIME, &_tick_start ); }

#define TIC( a )          {                                                    \
        clock_gettime ( CLOCK_REALTIME, &_tick_stop );                         \
        timespec diff;                                                         \
        diff.tv_sec  = _tick_stop.tv_sec - _tick_start.tv_sec;                 \
        diff.tv_nsec = _tick_stop.tv_nsec - _tick_start.tv_nsec;               \
        if ( diff.tv_nsec < 0 ) {                                              \
            diff.tv_sec  -= 1;                                                 \
            diff.tv_nsec += 1e9;                                               \
        }                                                                      \
        printf ( "%s: %lu s, %.2f ms\n", a, diff.tv_sec, diff.tv_nsec / 1e6 ); \
}





// List of supported commands.
const char * commands[] =
{
    "add",
    "edit",
    "view",
    "list",
    "delete",
    "projects",
    nullptr
};

/**
 * This project is written in C++, but tries to stick closer to C.
 * Classes, list, strings are ok.. Templates are already doubtful.
 */

class NotesFilter
{
private:
    std::vector<Note *> start_notes;
public:
    NotesFilter( std::vector< Note *> notes )
    {
        // Copy the list!
        start_notes = notes;
    }

    void add_filter ( std::string value )
    {
        for ( auto iter = start_notes.begin (); iter != start_notes.end (); iter++ ) {
            Note *note = *iter;

            // Skip empty elements.
            if ( note == nullptr ) {
                continue;
            }

            bool remove = true;

            if ( note->get_title ().rfind ( value ) != std::string::npos ) {
                remove = false;
            }
            if ( remove && note->get_project_name ().rfind ( value ) != std::string::npos ) {
                remove =
                    false;
            }

            if ( remove ) {
                *iter = nullptr;
            }
        }
    }

    const std::vector<Note *> &get_filtered_notes () const
    {
        return start_notes;
    }
};


// The Main object, this is also the root node.
class NotesCC : public Project
{
private:
    unsigned int        last_note_id = 0;
    std::string         db_path;
// Root project.
    std::vector<Note *> notes;


public:
    /**
     * This function is used to sort the notes and assign them an id.
     * Sorting should be stable between runs (when there is no edit of note).
     */
    static bool notes_sort ( Note *a, Note *b )
    {
        time_t diff_time = ( a->get_time_t () - b->get_time_t () );
        // If they are of equal time.
        if ( diff_time == 0 ) {
            int retv = a->get_title ().compare ( b->get_title () );
            if ( retv < 0 ) {
                return true;
            }
            // Sort on hash as a last resort, so we get a stable sort.
            return a->get_body_crc () - b->get_body_crc () > 0;
        }
        // If A is later then b it should go above a.
        return diff_time < 0;
    }
    NotesCC( const char *path ) : Project ( "" )
    {
        db_path = path;

        this->Load ( this, "" );

        std::sort ( this->notes.begin (), this->notes.end (), notes_sort );

        for ( auto note : this->notes ) {
            note->set_id ( ++this->last_note_id );
        }
    }
    ~NotesCC()
    {
        for ( auto note : notes ) {
            delete note;
        }
    }

    void print_projects ()
    {
        this->print ();
    }

    std::string get_path ()
    {
        return db_path;
    }

    void display_notes ( const std::vector<Note *> & view_notes )
    {
        TableView view;

        // Add the columns
        view.add_column ( "ID", color_bold );
        view.add_column ( "Rev.", color_blue );
        view.add_column ( "Project", color_white_bold );
        view.add_column ( "Last edited", color_green );
        view.add_column ( "Description" );

        unsigned int row_index = 0;
        for ( auto note : view_notes ) {
            // Skip empty elements.
            if ( note == nullptr ) {
                continue;
            }
            view[0].set_value ( row_index, std::to_string ( note->get_id () ) );
            if ( note->get_revision () > 0 ) {
                view[1].set_value ( row_index, std::to_string ( note->get_revision () ) );
            }
            else{
                view[1].set_value ( row_index, "" );
            }
            view[2].set_value ( row_index, note->get_project_name () );
            view[3].set_value ( row_index, note->get_modtime () );
            view[4].set_value ( row_index, note->get_title () );
            view++;
            row_index++;
        }
        view.print ();
    }

    /**
     * @param argc Number of renaming commandline options.
     * @param argv Remaining commandline options.
     *
     * Edit a notes.
     *
     * @returns number of consumed commandline options.
     */
    int command_edit ( int argc, char ** argv )
    {
        // TODO: abstract this in a 'get note'
        int cargs = 0;
        if ( argc <= 0 ) {
            fprintf ( stderr, "edit requires one argument\n" );
            return cargs;
        }

        cargs++;
        int nindex = std::stoi ( argv[0] );
        if ( nindex < 1 || nindex > (int) notes.size () ) {
            fprintf ( stderr, "Invalid note id: %d\n", nindex );
            return cargs;
        }
        Note *note = notes[nindex - 1];

        // Edit the note.
        note->edit ();

        return cargs;
    }

    /**
     * @param argc Number of renaming commandline options.
     * @param argv Remaining commandline options.
     *
     * View a notes.
     *
     * @returns number of consumed commandline options.
     */
    int command_view ( int argc, char ** argv )
    {
        // TODO: abstract this in a 'get note'
        // Note *note = this->get_note (argc[in], argv[in], cargs[out]);
        int cargs = 0;
        if ( argc <= 0 ) {
            fprintf ( stderr, "view requires one argument\n" );
            return cargs;
        }

        cargs++;
        int nindex = std::stoi ( argv[0] );
        if ( nindex < 1 || nindex > (int) notes.size () ) {
            fprintf ( stderr, "Invalid note id: %d\n", nindex );
            return cargs;
        }
        Note *note = notes[nindex - 1];

        note->view ();



        return cargs;
    }


    /**
     * @param argc Number of renaming commandline options.
     * @param argv Remaining commandline options.
     *
     * List the notes.
     *
     * @returns number of consumed commandline options.
     */
    int command_list ( int argc, char ** argv )
    {
        int         iter = 0;
        NotesFilter filter ( this->notes );

        for (; iter < argc; iter++ ) {
            filter.add_filter ( argv[iter] );
        }
        this->display_notes ( filter.get_filtered_notes () );
        return iter;
    }

    static void command_projects_add_entry ( Project *p, TableView &view, unsigned int &row )
    {
        view[0].set_value ( row, p->get_name () );
        std::string nnotes =
            std::to_string ( p->get_num_notes () ) + "/" + std::to_string ( p->get_num_notes_recursive () );
        view[1].set_value ( row, nnotes );
        row++;
        view++;
        for ( auto pc : p->get_child_projects () ) {
            command_projects_add_entry ( pc, view, row );
        }
    }
    int command_projects ( int argc, char **argv )
    {
        TableView view;
        view.add_column ( "Project", color_blue );
        view.add_column ( "Num. Notes", color_white );
        unsigned int row = 0;
        for ( auto p : this->get_child_projects () ) {
            command_projects_add_entry ( p, view, row );
        }
        view.print ();
    }

    void command_view_autocomplete ()
    {
        for ( auto note : notes ) {
            printf ( "%u\n", note->get_id () );
        }
    }

    void command_edit_autocomplete ()
    {
        for ( auto note : notes ) {
            printf ( "%u\n", note->get_id () );
        }
    }


    /**
     * Implement the autocomplete command.
     */
    int autocomplete ( int argc, char **argv )
    {
        if ( argc == 0 ) {
            // List commands.
            for ( int i = 0; commands[i] != nullptr; i++ ) {
                std::cout << commands[i] << std::endl;
            }
            return 0;
        }
        if ( strcasecmp ( argv[0], "view" ) == 0 ) {
            this->command_view_autocomplete ();
            return 1;
        }
        else if ( strcasecmp ( argv[0], "edit" ) == 0 ) {
            this->command_edit_autocomplete ();
            return 1;
        }
        this->list_projects ();

        return 1;
    }

    void run ( int argc, char **argv )
    {
        int index = 1;
        while ( index < argc ) {
            if ( strcmp ( argv[index], "--complete" ) == 0 ) {
                index++;
                index += this->autocomplete ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "view" ) == 0 ) {
                index++;
                index += this->command_view ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "list" ) == 0 ) {
                index++;
                index += this->command_list ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "edit" ) == 0 ) {
                index++;
                index += this->command_edit ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "projects" ) == 0 ) {
                index++;
                index += this->command_projects ( argc - index, &argv[index] );
            }
            else {
                fprintf ( stderr, "Invalid argument: %s\n", argv[index] );
                return;
            }
        }
    }

private:
    void Load ( Project *node, std::string path )
    {
        DIR *dir = opendir ( ( db_path + path ).c_str () );
        if ( dir != NULL ) {
            struct dirent *dirp;
            while ( ( dirp = readdir ( dir ) ) != NULL ) {
                // Skip hidden files (for now)
                if ( dirp->d_name[0] == '.' ) {
                    continue;
                }
                // Project
                if ( dirp->d_type == DT_DIR ) {
                    Project *p = new Project ( dirp->d_name );
                    node->add_subproject ( p );

                    // Recurse down in the structure.
                    std::string np = path + "/" + dirp->d_name;
                    Load ( p, np );
                }
                // Note
                else if ( dirp->d_type == DT_REG ) {
                    Note *note = new Note ( node, dirp->d_name );
                    // Add to the flat list in the main.
                    this->notes.push_back ( note );
                    node->add_note ( note );
                }
            }
            closedir ( dir );
        }
    }
};


int main ( int argc, char ** argv )
{
    char *path = NULL;
    INIT_TIC_TAC ()

    rhash_library_init ();

    if ( asprintf ( &path, "%s/Notes2/", getenv ( "HOME" ) ) == -1 ) {
        fprintf ( stderr, "Failed to get path\n" );
        return EXIT_FAILURE;
    }
    NotesCC notes ( path );

    notes.run ( argc, argv );

    free ( path );
    if ( argc < 2 || strcasecmp ( argv[1], "--complete" ) ) {
        TIC ( "finish" );
    }
    return EXIT_SUCCESS;
}
