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
typedef enum _MainCommands
{
    MC_ADD,
    MC_EDIT,
    MC_VIEW,
    MC_LIST,
    MC_DELETE,
    MC_NUM
} MainCommands;
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
    unsigned int      last_note_id = 0;
    std::string       db_path;
// Root project.
    std::list<Note *> notes;


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

        this->notes.sort ( notes_sort );
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


    int autocomplete ( int argc, char **argv )
    {
        if ( argc == 0 ) {
            // List commands.
            for ( int i = 0; commands[i] != nullptr; i++ ) {
                std::cout << commands[i] << std::endl;
            }
            return 0;
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
            else if ( strcmp ( argv[index], "list" ) == 0 ) {
                TableView view;

                // Add the columns
                view.add_column ( "ID", color_bold );
                view.add_column ( "Project", color_white_bold );
                view.add_column ( "Last edited", color_blue );
                view.add_column ( "CRC", color_red );
                view.add_column ( "Description" );
                for ( auto note : notes ) {
                    unsigned int row_index = note->get_id () - 1;
                    view[0].set_value ( row_index, std::to_string ( note->get_id () ) );
                    view[1].set_value ( row_index, note->get_project () );
                    view[2].set_value ( row_index, note->get_modtime () );
                    view[3].set_value ( row_index, std::to_string ( note->get_body_crc () ) );
                    view[4].set_value ( row_index, note->get_title () );
                    view++;
                }
                view.print ();
                index++;
            }
            else {
                std::cerr << "Invalid argument: " << argv[index] << std::endl;
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

    rhash_library_init ();

    if ( asprintf ( &path, "%s/Notes2/", getenv ( "HOME" ) ) == -1 ) {
        fprintf ( stderr, "Failed to get path\n" );
        return EXIT_FAILURE;
    }
    INIT_TIC_TAC ()
    NotesCC notes ( path );

    notes.run ( argc, argv );

    TIC ( "finish" );
    free ( path );
    return EXIT_SUCCESS;
}
