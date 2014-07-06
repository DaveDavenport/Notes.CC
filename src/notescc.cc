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
    nullptr
};

/**
 * This project is written in C++, but tries to stick closer to C.
 * Classes, list, strings are ok.. Templates are already doubtful.
 */


// The Main object, this is also the root node.
class NotesCC : public Project
{
private:
    unsigned int        last_note_id = 0;
    std::string         db_path;
// Root project.
    std::vector<Note *> notes;


public:
    static bool notes_sort ( Note *a, Note *b )
    {
        int time = ( a->get_time_t () - b->get_time_t () );
        if ( time == 0 ) {
            return a->get_title ().compare ( b->get_title () ) < 0;
            // TODO sort on filename last resort, so we get a stable sort.
        }
        return time < 0;
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

    void display_notes ( std::vector<Note *> & view_notes )
    {
        TableView view;

        // Add the columns
        view.add_column ( "ID", color_bold );
        view.add_column ( "Project", color_white_bold );
        view.add_column ( "Last edited", color_blue );
        view.add_column ( "CRC", color_red );
        view.add_column ( "Description" );

        // TODO: Add filter.
        // NoteFilter filter();
        // for ( auto arg : argv ) { filter.push_element(); }

        for ( auto note : view_notes ) {
            // if ( filter.skip_note(note) ) continue;
            unsigned int row_index = note->get_id () - 1;
            view[0].set_value ( row_index, std::to_string ( note->get_id () ) );
            view[1].set_value ( row_index, note->get_project () );
            view[2].set_value ( row_index, note->get_modtime () );
            view[3].set_value ( row_index, std::to_string ( note->get_body_crc () ) );
            view[4].set_value ( row_index, note->get_title () );
            view++;
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
        this->display_notes ( this->notes );
        return 0;
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
        // In theory we could concatenate commands.. Disabled for now
        // TODO: See how to handle concatenation, esp with things like
        // edit.
        if ( index < argc ) {
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
