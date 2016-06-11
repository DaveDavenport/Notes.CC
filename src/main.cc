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
/**
 * TODO: make NotesCC::notes a map<id, Note *> instead of list?
 */

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <list>
#include <map>

#include <Project.h>
#include <Note.h>
#include <Colors.h>
#include <TableView.h>
#include <IDStorage.h>
#include <Filter.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

// Readline
#include <readline/readline.h>
#include <readline/history.h>

#include <Settings.h>

struct timespec _tick_start;
struct timespec _tick_stop;

#define INIT_TIC_TAC()    { clock_gettime ( CLOCK_REALTIME, &_tick_start ); }

#define TIC( a )          {                                                              \
        clock_gettime ( CLOCK_REALTIME, &_tick_stop );                                   \
        timespec diff;                                                                   \
        diff.tv_sec  = _tick_stop.tv_sec - _tick_start.tv_sec;                           \
        diff.tv_nsec = _tick_stop.tv_nsec - _tick_start.tv_nsec;                         \
        if ( diff.tv_nsec < 0 ) {                                                        \
            diff.tv_sec  -= 1;                                                           \
            diff.tv_nsec += 1e9;                                                         \
        }                                                                                \
        notes_print_info ( "%s: %lu s, %.2f ms\n", a, diff.tv_sec, diff.tv_nsec / 1e6 ); \
}





// List of supported commands.
const char * commands[] =
{
    "add",
    "move",
    "edit",
    "view",
    "cat",
    "list",
    "export",
    "import",
    "delete",
    "projects",
    "interactive",
    "pull",
    "push",
    "sync",
    "archive",
    nullptr
};

/**
 * This project is written in C++, but tries to stick closer to C.
 * Classes, list, strings are ok.. Templates only when it safes duplication.
 */





// The Main object, this is also the root node.
class NotesCC : public Project
{
private:
// Root project.
    std::vector<Note *> notes;

    bool                require_sync = false;
    bool                git_changed  = false;
    IDStorage           *storage     = nullptr;
    Settings            settings;

    // Output settings.
    bool show_archive = false;

public:
    NotesCC( )  : Project ( "" )
    {
    }
    ~NotesCC()
    {
        // Commit lingering changes.
        if ( git_changed ) {
            notes_print_info ( "Commiting changes to git.\n" );
            repository_commit_changes ();
            require_sync = true;
            git_changed  = false;
        }
        // Push it when needed.
        //    @TODO add message sync needed.

        if ( storage != nullptr ) {
            delete storage;
            storage = nullptr;
        }

        // Clear internal state.
        clear ();
        if ( require_sync ) {
            notes_print_info ( color_yellow "Sync required\n" color_reset );
        }
    }
    void pre_parse_settings ( int &argc, char **argv )
    {
        // Set quiet.
        if ( argc > 1 && strcmp ( argv[1], "--complete" ) == 0 ) {
            notes_print_quiet ();
        }
        for ( int index = 1; index < argc; index++ ) {
            if ( strcmp ( argv[index], "--repo" ) == 0 && argc > ( index + 1 ) ) {
                std::string repo_path = argv[index + 1];

                settings.set_repository ( repo_path );
                memmove ( &argv[index], &argv[index + 2], sizeof ( char* ) * ( argc - index ) );
                argc  -= 2;
                index -= 2;
            }
        }
    }

    void run ( int argc, char **argv )
    {
        // Check interactive mode.
        if ( argc == 2 && strcmp ( argv[1], "interactive" ) == 0 ) {
            notes_print_info ( "Interactive mode\n" );
            interactive ();
        }
        else if  ( argc >= 2 && strcmp ( argv[1], "sync" ) == 0 ) {
            this->repository_pull ();
            this->repository_push ();
        }
        else if ( argc >= 2 && strcmp ( argv[1], "pull" ) == 0 ) {
            this->repository_pull ();
        }
        else if ( argc >= 2 && strcmp ( argv[1], "push" ) == 0 ) {
            this->repository_push ();
        }

        // Check autocomplete.
        else if ( argc > 1 && strcmp ( argv[1], "--complete" ) == 0 ) {
            run_autocomplete ( argc - 1, &argv[1] );
        }

        // Commandline parser
        else {
            cmd_parser ( argc - 1, &argv[1] );
        }
    }

    bool initial_setup ()
    {
        std::string repo_path = this->get_path ();
        // Do intro
        notes_print_error ( "Do you want Notes.CC to initialize this directory?\n" );
        char *response = readline ( "(y/n): " );
        if ( response && strcmp ( response, "y" ) == 0 ) {
            // Try to make directory.
            if ( mkdir ( repo_path.c_str (), 0750 ) != 0 ) {
                if ( errno != EEXIST ) {
                    notes_print_error ( "Failed to create directory: %s\n", strerror ( errno ) );
                    return true;
                }
            }
            const char *const args[] = { "git", "-C", repo_path.c_str (), "init", NULL };
            auto              cur    = exec_command ( "git", args );
            if ( cur != 0  ) {
                return true;
            }
        }
        {
            const char * const args[] = { "git", "-C", repo_path.c_str (), "config", "user.name", NULL };
            auto               val    = exec_command_read_result ( "git", args );
            if ( val.size () == 0 ) {
                const char * const args2[] = { "git", "-C", repo_path.c_str (), "config", "user.name", "Notes.CC", NULL };
                exec_command ( "git", args2 );
            }
        }
        {
            const char *const args[] = { "git", "-C", repo_path.c_str (), "config", "user.email", NULL };
            auto              val    = exec_command_read_result ( "git", args );
            if ( val.size () == 0 ) {
                const char *const args2[] = { "git", "-C", repo_path.c_str (), "config", "user.email", "note@Notes.CC", NULL };
                exec_command ( "git", args2 );
            }
        }
        /** Check for user.name and user.email */
        return false;
    }

    /**
     * Open the note repository.
     */
    bool open_repository ( )
    {
        std::string       repo_path = this->get_path ();
        const char *const argsc[]   = { "git",
                                        "-C",                   repo_path.c_str (),
                                        "rev-parse",
                                        "--is-inside-work-tree",
                                        NULL };
        auto              isdir = exec_command_read_result ( "git", argsc );
        if ( strcasecmp ( isdir.c_str (), "true" ) != 0 ) {
            notes_print_error ( "Notes repository is not a git directory\n" );
            if ( initial_setup () ) {
                notes_print_error ( "Failed to initialize, giving up.\n" );
                return false;
            }
        }
        // @TODO  check for .git directory. (or ask git?)
        // Create the ID storage->
        if ( storage == nullptr ) {
            storage = new IDStorage ( settings.get_repository () );
        }
        const char *const args[] = { "git",
                                     "-C",       repo_path.c_str (),
                                     "rev-parse",
                                     "@{0}",
                                     NULL };
        auto              cur     = exec_command_read_result ( "git", args );
        const char *const args1[] = { "git",
                                      "-C",       repo_path.c_str (),
                                      "rev-parse",
                                      "@{u}",
                                      NULL };
        auto              up = exec_command_read_result ( "git", args1 );
        if ( up.size () == 0 ) {
            // this->has_upstream = false;
        }
        else if ( cur != up ) {
            this->require_sync = true;
        }
        this->Load ( );
        return true;
    }

private:
    /**
     * This function is used to sort the notes and assign them an id.
     * Sorting should be stable between runs (when there is no edit of note).
     */
    static bool notes_print_sort ( Note *a, Note *b )
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

    /**
     * Repository interaction functions.
     */
    int exec_command ( const char *cmd, const char *const args[] )
    {
        pid_t pid;

        switch ( pid = vfork () )
        {
        case -1:    /* Error */
            notes_print_error ( "Failed to run %s: %s\n", cmd, strerror ( errno ) );
            return 1;
        case 0:     /* child */
            int i;
            {
                int na = 0;
                for (; args[na] != nullptr; na++ ) {
                    ;
                }
                char **vargs = (char * *) malloc ( ( na + 1 ) * sizeof ( char * ) );
                for ( int i = 0; i < na; i++ ) {
                    vargs[i]     = strdup ( args[i] );
                    vargs[i + 1] = nullptr;
                }
                execvp ( cmd, vargs );
                printf ( "Error: %s\n", strerror ( errno ) );
                for ( int i = 0; i < na; i++ ) {
                    free ( vargs[i] );
                }
                free ( vargs );
            }
            _exit ( 127 );
        default:
            break;
        }
        // Wait until client is done.
        if ( pid > 0 ) {
            waitpid ( pid, NULL, 0 );
        }
        return 0;
    }
    std::string exec_command_read_result ( const char *cmd, const char *const args[] )
    {
        pid_t pid;
        int   filedes[2];
        if ( pipe2 ( filedes, 0 ) == -1 ) {
            perror ( "pipe" );
            exit ( 1 );
        }
        pid = fork ();
        switch ( pid )
        {
        case -1:        /* Error */
            notes_print_error ( "Failed to run %s: %s\n", cmd, strerror ( errno ) );
            return "";
        case 0:         /* child */
            int i;
            {
                close ( filedes[0] );
                // ignore stderr.
                close ( STDERR_FILENO );
                while ( ( dup2 ( filedes[1], STDOUT_FILENO ) == -1 ) && ( errno == EINTR ) ) {
                }
                int na = 0;
                for (; args[na] != nullptr; na++ ) {
                    ;
                }
                char **vargs = (char * *) malloc ( ( na + 1 ) * sizeof ( char * ) );
                for ( int i = 0; i < na; i++ ) {
                    vargs[i]     = strdup ( args[i] );
                    vargs[i + 1] = nullptr;
                }
                execvp ( cmd, vargs );
                for ( int i = 0; i < na; i++ ) {
                    free ( vargs[i] );
                }
                free ( vargs );
            }
            _exit ( 127 );
        default:
            close ( filedes[1] );
            break;
        }
        // Wait until client is done.
        if ( pid > 0 ) {
            waitpid ( pid, NULL, 0 );
        }
        std::string retv;
        char        buffer[1024];
        while ( 1 ) {
            ssize_t count = read ( filedes[0], buffer, 1024 );
            if ( count == -1 ) {
                if ( errno == EINTR ) {
                    continue;
                }
                else {
                    perror ( "read" );
                    break;
                }
            }
            else if ( count == 0 ) {
                break;
            }
            else {
                buffer[count] = '\0';
                retv         += buffer;
            }
        }
        close ( filedes[0] );
        // Strip newline.
        if ( retv[retv.size () - 1] == '\n' ) {
            retv[retv.size () - 1] = '\0';
        }
        return retv;
    }

    void repository_delete_file ( std::string path )
    {
        std::string       repo_path = this->get_path ();
        notes_print_info ( "Delete file: %s\n", path.c_str () );
        const char *const args[] = { "git",
                                     "-C",         repo_path.c_str (),
                                     "rm",         "--cached",
                                     path.c_str (),
                                     NULL };
        int               retv = exec_command ( "git", args );
        if ( retv != 0 ) {
            notes_print_error ( "Failed add changes to index.\n" );
        }
        git_changed = true;
    }
    void repository_stage_file ( std::string path )
    {
        notes_print_info ( "Staging file: %s\n", path.c_str () );

        std::string       repo_path = this->get_path ();
        const char *const args[]    = { "git",
                                        "-C",    repo_path.c_str (),
                                        "add",   path.c_str (),
                                        NULL };
        int               retv = exec_command ( "git", args );
        if ( retv != 0 ) {
            notes_print_error ( "Failed add changes to index.\n" );
        }
        git_changed = true;
    }

    bool repository_commit_changes ( )
    {
        std::string       repo_path = this->get_path ();
        const char *const args[]    = { "git",
                                        "-C",    repo_path.c_str (),
                                        "commit",
                                        "-m",    "Updates",
                                        NULL };
        int               retv = exec_command  ( "git", args );
        if ( retv != 0 ) {
            notes_print_error ( "Failed commit\n" );
            return false;
        }
        return true;
    }

    bool repository_push ()
    {
        std::string       path   = this->get_path ();
        const char *const args[] = { "git", "-C", path.c_str (), "push", NULL };
        int               retv   = exec_command ( "git", args );
        return retv == 0;
    }
    bool repository_pull ( )
    {
        std::string       path   = this->get_path ();
        const char *const args[] = { "git", "-C", path.c_str (), "pull", NULL };
        int               retv   = exec_command ( "git", args );
        if ( retv == 0 ) {
            // TODO: check return value.
            this->clear ();
            this->Load ();
        }
        return retv == 0;
    }


    void clear ()
    {
        // delete notes.
        for ( auto note : notes ) {
            if ( note != nullptr ) {
                delete note;
            }
        }
        notes.clear ();
        for ( auto project : child_projects ) {
            delete project;
        }
        child_projects.clear ();
    }


    void print_projects ()
    {
        this->print ();
    }

    std::string get_path ()
    {
        return settings.get_repository ();
    }

    std::string get_relative_path ()
    {
        return "";
    }


    /**
     * Get the index based on id.
     */
    int get_note_index ( unsigned int id )
    {
        for ( unsigned int index = 0; index < notes.size (); index++ ) {
            if ( notes[index]->get_id () == id ) {
                return index;
            }
        }
        return -1;
    }

    template < class T>
    void display_notes ( const T view_notes )
    {
        TableView view;

        // Add the columns
        view.add_column ( "ID", color_bold );
        view[0].set_right_align ();
        view.add_column ( "Rev.", color_blue );
        view[1].set_right_align ();
        view.add_column ( "Project", color_magenta );
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



    Note * get_note ( int argc, char ** argv )
    {
        if ( argc == 1 ) {
            // Get index.
            try {
                unsigned int note_id = std::stoul ( argv[0] );
                int          nindex  = this->get_note_index ( note_id );
                if ( nindex >= 0 ) {
                    return notes[nindex];
                }
                notes_print_error ( "Invalid note id: %d\n", nindex );
            } catch ( ... ) {
            }
        }

        if ( argc == 1 ) {
            // Get the last note.
            if ( strcasecmp ( argv[0], "last" ) == 0 ) {
                if ( this->notes.size () > 0 ) {
                    return this->notes[this->notes.size () - 1];
                }
            }
        }


        NotesFilter filter ( this->notes );
        for ( int iter = 0; iter < argc; iter++ ) {
            filter.add_filter ( argv[iter] );
        }

        // Get filtered notes.
        auto fnotes = filter.get_filtered_notes ();
        if ( fnotes.size () == 0 ) {
            return nullptr;
        }

        // If one note is remaining, pick that one
        else if ( fnotes.size () == 1 ) {
            return *( fnotes.begin () );
        }

        while ( true ) {
            this->display_notes ( fnotes );

            char* resp = readline ( "Enter note id: " );
            if ( resp ) {
                // Quit
                if ( resp[0] == 'q' ) {
                    free ( resp );
                    return nullptr;
                }
                try {
                    unsigned int note_id = std::stoul ( resp );
                    int          nindex  = this->get_note_index ( note_id );
                    if ( nindex >= 0  ) {
                        free ( resp );
                        return this->notes[nindex];
                    }
                }catch ( ... ) { }

                notes_print_error ( "Invalid note id: %s\n", resp );
                free ( resp );
            }
        }
        return nullptr;
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
        Note *note = this->get_note ( argc, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note selected\n" );
            return argc;
        }
        // Edit the note.
        if ( note->edit () ) {
            // Commit the result.
            auto path = note->get_relative_path ();
            repository_stage_file ( path );
        }

        return argc;
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
        Note *note = this->get_note ( argc, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note selected\n" );
            return argc;
        }

        note->view ();
        return argc;
    }

    /**
     * @param argc Number of renaming commandline options.
     * @param argv Remaining commandline options.
     *
     * Print the raw note to stdout.
     *
     * @returns number of consumed commandline options.
     */
    int command_cat ( int argc, char ** argv )
    {
        Note *note = this->get_note ( argc, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note selected\n" );
            return argc;
        }

        note->cat ();
        return argc;
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

        // Ignore archived notes.
        if ( show_archive ) {
            filter.filter_not_archive ();
        }
        else {
            filter.filter_archive ();
        }
        if ( argc > 0 ) {
            for (; iter < argc; iter++ ) {
                filter.add_filter ( argv[iter] );
            }
        }
        this->display_notes ( filter.get_filtered_notes () );
        notes_print_info ( "Number notes: %lu\n", filter.get_filtered_notes ().size () );
        return iter;
    }
    int command_export ( int argc, char **argv )
    {
        if ( argc != 3 ) {
            notes_print_error ( "Export requires three arguments: <note id> <type> <export_file>\n" );
            return 0;
        }

        Note *note = get_note ( 1, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note specified\n" );
            return 3;
        }

        std::string export_path = argv[2];
        std::string format      = argv[1];

        notes_print_info ( "Exporting note %u to '%s'\n", note->get_id (), argv[2] );

        if ( format == "raw" ) {
            note->export_to_file_raw ( export_path );
        }
        else if ( format == "html" ) {
            note->export_to_file_html ( export_path );
        }
        else {
            notes_print_error ( "%s is an invalid export format. (raw and html are supported)\n",
                                format.c_str () );
        }
        return 3;
    }

    int command_import ( int argc, char **argv )
    {
        if ( argc < 1 ) {
            notes_print_error ( "Import requires atleast one arguments:  <import_file> (<Project>)\n" );
            return argc;
        }
        Project *p = this;
        if ( argc >= 2 ) {
            std::string name = argv[1];
            p = this->get_or_create_project_from_name ( name );
            if ( p == nullptr ) {
                return 2;
            }
        }
        std::string path = argv[0];
        Note        *n   = new Note ( p, &settings );

        if ( n != nullptr ) {
            if ( !n->import ( path ) ) {
                // Failed, clean up again.
                n->del ();
                delete n;
                return 2;
            }
            // Success, add it.
            this->notes.push_back ( n );
            n->set_id ( storage->get_id ( n->get_relative_path () ) );
            auto path = n->get_relative_path ();
            repository_stage_file ( path );
        }
        return 2;
    }

private:

    int command_move ( int argc, char **argv )
    {
        if ( argc < 2 ) {
            notes_print_error ( "Move requires two arguments: <note id>  <project path>\n" );
            return argc;
        }

        Note *note = get_note ( 1, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note specified\n" );
            return 2;
        }

        std::string name = argv[1];
        Project     *p   = this->get_or_create_project_from_name ( name );
        if ( p == nullptr ) {
            return 2;
        }
        if ( p == note->get_project () ) {
            notes_print_warning ( "Destination same as source: Ignoring.\n" );
            return 2;
        }
        std::string old_path = note->get_relative_path ();
        if ( note->move ( p ) ) {
            std::string new_path = note->get_relative_path ();
            this->repository_stage_file ( new_path );
            this->repository_delete_file ( old_path );
            // Move id
            this->storage->move_id ( old_path, new_path );
        }
        return 2;
    }

    static void command_projects_add_entry ( Project *p, TableView &view, unsigned int &row )
    {
        view[0].set_value ( row, p->get_name () );
        view[1].set_value ( row, std::to_string ( p->get_num_notes () ) );
        view[2].set_value ( row, std::to_string ( p->get_num_notes_recursive () ) );
        row++;
        view++;
        for ( auto pc : p->get_child_projects () ) {
            command_projects_add_entry ( pc, view, row );
        }
    }
    int command_projects ( __attribute__( ( unused ) ) int argc, __attribute__( ( unused ) ) char **argv )
    {
        TableView view;
        view.add_column ( "Project" );
        view.add_column ( "Num. Notes", color_blue );
        view.add_column ( "Total Notes", color_magenta );
        view[1].set_right_align ();
        view[2].set_right_align ();
        unsigned int row = 0;
        for ( auto pc : this->get_child_projects () ) {
            // Filter out archive.
            if ( ( pc->get_name () == "Archive" ) == show_archive ) {
                command_projects_add_entry ( pc, view, row );
            }
        }
        view.print ();
        notes_print_info ( "Number projects: %lu\n", row );
        return 0;
    }

    void autocomplete_list_notes_ids_and_keywords ()
    {
        for ( auto note : notes ) {
            printf ( "%u\n", note->get_id () );
        }
        if ( notes.size () > 0 ) {
            printf ( "last\n" );
        }
    }

    void command_view_autocomplete ()
    {
        autocomplete_list_notes_ids_and_keywords ();
    }

    void command_edit_autocomplete ()
    {
        autocomplete_list_notes_ids_and_keywords ();
    }

    /**
     * Resolve a normal full project name into the Project pointer.
     * If (sub) projects do not exists, they are created and added.
     *
     * Before adding a note, validate if it exists by calling
     * Project::check_and_create_path().
     */
    Project * get_or_create_project_from_name ( std::string pr )
    {
        Project *p = this;
        if ( pr.empty () ) {
            return p;
        }
        // Split the string.
        size_t str_start = 0;
        while ( str_start < pr.size () ) {
            auto str_end = pr.find_first_of ( '.', str_start );
            if ( str_end == std::string::npos ) {
                str_end = pr.size ();
            }
            auto pr_name = pr.substr ( str_start, str_end - str_start );
            if ( !pr_name.empty () ) {
                // Validate the Project name only consists of characters and numbers.
                if ( find_if ( pr_name.begin (), pr_name.end (),
                               [] ( char c ) {
                                   return !( isalnum ( c ) );
                               } ) != pr_name.end () ) {
                    notes_print_error ( "%s is an invalid Project name.\n", pr_name.c_str () );
                    return nullptr;
                }
                Project *pc = p->find_child ( pr_name );
                if ( pc == nullptr ) {
                    // Create!
                    pc = new Project ( pr_name.c_str () );
                    p->add_subproject ( pc );
                }
                p = pc;
            }

            str_start = str_end + 1;
        }
        return p;
    }
    int command_archive_move ( int argc, char ** argv )
    {
        Note *note = this->get_note ( argc, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note selected\n" );
            return argc;
        }
        std::string name = "Archive." + note->get_project_name ();
        Project     *p   = this->get_or_create_project_from_name ( name );
        if ( p == nullptr ) {
            notes_print_error ( "Failed to create project by name: %s", name.c_str () );
            return argc;
        }
        if ( p == note->get_project () ) {
            notes_print_warning ( "Destination same as source: Ignoring.\n" );
            return argc;
        }
        std::string old_path = note->get_relative_path ();
        if ( note->move ( p ) ) {
            std::string new_path = note->get_relative_path ();
            this->repository_stage_file ( new_path );
            this->repository_delete_file ( old_path );
            // Move id
            this->storage->move_id ( old_path, new_path );
        }
        return argc;
    }
    int command_archive_restore ( int argc, char ** argv )
    {
        Note *note = this->get_note ( argc, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note selected\n" );
            return argc;
        }
        const Project *rp = note->get_project ();
        while ( !rp->is_root () ) {
            if ( rp->get_name () == "Archive" ) {
                break;
            }
            rp = rp->get_parent ();
        }
        if ( rp->is_root () ) {
            notes_print_error ( "Note is not archived, cannot restore\n" );
            return argc;
        }
        std::string name     = note->get_relative_project_name ( rp );
        Project     *p       = this->get_or_create_project_from_name ( name );
        std::string old_path = note->get_relative_path ();
        if ( note->move ( p ) ) {
            std::string new_path = note->get_relative_path ();
            this->repository_stage_file ( new_path );
            this->repository_delete_file ( old_path );
            // Move id
            this->storage->move_id ( old_path, new_path );
        }
        return argc;
    }
    int command_archive ( int argc, char **argv )
    {
        show_archive = true;
        if ( argc == 0 ) {
            return this->command_list ( argc, argv );
        }
        if ( strcmp ( argv[0], "move" ) == 0 ) {
            return 1 + command_archive_move ( argc - 1, &argv[1] );;
        }
        else if ( strcmp ( argv[0], "restore" ) == 0 ) {
            return 1 + command_archive_restore ( argc - 1, &argv[1] );;
        }
        return 0;
    }

    int command_delete ( int argc, char **argv )
    {
        Note *note = this->get_note ( argc, argv );
        if ( note == nullptr ) {
            notes_print_error ( "No note selected\n" );
            return argc;
        }

        // Delete the file from internal structure and working directory.
        int nindex = this->get_note_index ( note->get_id () );

        if ( nindex < 0 ) {
            notes_print_error ( "Internal error, note with id: %u does not exist.\n",
                                nindex );
            return argc;
        }

        notes_print_warning ( "Are you sure you want to delete note with title: '%s'\n",
                              note->get_title ().c_str () );
        notes_print_warning ( "Press 'a' to archive note instead.\n" );
        char *response = readline ( "(y/n/a): " );
        if ( response && strcmp ( response, "y" ) == 0 ) {
            notes_print_warning ( "Deleting note\n" );
            std::string path = note->get_relative_path ();
            if ( note->del () ) {
                // Tell git the file is removed.
                repository_delete_file ( path );
                storage->delete_id ( path );
                // Delete the entry from the list.
                delete note;
                notes[nindex] = nullptr;
            }
        }
        else if ( response && strcmp ( response, "a" ) == 0 ) {
            std::string name = "Archive." + note->get_project_name ();
            Project     *p   = this->get_or_create_project_from_name ( name );
            if ( p == nullptr ) {
                notes_print_error ( "Failed to create project by name: %s", name.c_str () );
                return argc;
            }
            if ( p == note->get_project () ) {
                notes_print_warning ( "Destination same as source: Ignoring.\n" );
                return argc;
            }
            std::string old_path = note->get_relative_path ();
            if ( note->move ( p ) ) {
                std::string new_path = note->get_relative_path ();
                this->repository_stage_file ( new_path );
                this->repository_delete_file ( old_path );
                // Move id
                this->storage->move_id ( old_path, new_path );
            }
        }
        else{
            notes_print_info ( "Aborting delete.\n" );
        }
        free ( response );
        return argc;
    }
    int command_add ( int argc, char **argv )
    {
        int     retv = 0;
        Project *p   = this;

        if ( argc > 0 ) {
            p = this->get_or_create_project_from_name ( argv[0] );
            retv++;
        }

        // Check if we have project successful.
        if ( p == nullptr ) {
            notes_print_error ( "Failed to find or create the project.\n" );
            return retv;
        }

        Note *n = new Note ( p, &settings );

        if ( n != nullptr ) {
            this->notes.push_back ( n );
            n->set_id ( storage->get_id ( n->get_relative_path () ) );
            n->edit ();
            // Commit the result.
            auto path = n->get_relative_path ();
            repository_stage_file ( path );
        }

        return retv;
    }

    void command_export_autocomplete ( int argc, __attribute__( ( unused ) ) char **argv  )
    {
        if ( argc == 1 ) {
            autocomplete_list_notes_ids_and_keywords ();
        }
        else if ( argc == 2 ) {
            printf ( "html\nraw\n" );
        }
    }
    void command_move_autocomplete ( int argc, __attribute__ ( ( unused ) ) char **argv )
    {
        if ( argc == 1 ) {
            autocomplete_list_notes_ids_and_keywords ();
        }
        else if ( argc == 2 ) {
            list_projects ();
        }
    }

    /**
     * Implement the autocomplete command.
     */
    void run_autocomplete ( int argc, char **argv )
    {
        if ( argc == 1 ) {
            // List commands.
            for ( int i = 0; commands[i] != nullptr; i++ ) {
                std::cout << commands[i] << std::endl;
            }
            return;
        }
        std::string command = argv[1];
        if ( command == "view" || command == "cat" ) {
            if ( argc == 2 ) {
                this->command_view_autocomplete ();
            }
            return;
        }
        else if ( command == "archive" ) {
            if ( argc >= 3 && std::string ( argv[2] ) == "move" ) {
                if ( argc == 3 ) {
                    autocomplete_list_notes_ids_and_keywords ();
                }
                return;
            }
            else if ( argc >= 3 && std::string ( argv[2] ) == "restore" ) {
                if ( argc == 3 ) {
                    autocomplete_list_notes_ids_and_keywords ();
                }
                return;
            }
            // Archive has restore command.
            std::cout << "restore" << std::endl;
            // This is now just a modifier.
            run_autocomplete ( argc - 1, &argv[1] );
        }
        else if ( command == "export" ) {
            this->command_export_autocomplete ( argc - 1, &argv[1] );
        }
        else if ( command == "import" ) {
            if ( argc == 3 ) {
                this->list_projects ();
            }
            return;
        }
        else if ( command == "move" ) {
            this->command_move_autocomplete ( argc - 1, &argv[1] );
        }
        else if ( command == "edit"  ) {
            if ( argc == 2 ) {
                this->command_edit_autocomplete ();
            }
            return;
        }
        else if ( command == "add" ) {
            if ( argc == 2 ) {
                this->list_projects ();
            }
            return;
        }
    }

    /**
     * Interactive shell
     */
    void interactive ()
    {
        // Enable history.
        using_history ();

        do {
            // In interactive mode we want to make sure the list is always nicely ordered.
            // This is needed so commands like 'last' are always consistent.
            sort_notes ();

            // Create interactive prompt.
            char *temp = readline ( "> " );

            // Quit on ctrl-d or quit.
            if ( temp == nullptr ) {
                break;
            }
            if ( strcasecmp ( temp, "quit" ) == 0 ) {
                free ( temp );
                break;
            }
            // Add entry to history.
            add_history ( temp );

            // Split into arc, argv structure.
            int  length     = strlen ( temp );
            int  argc       = 0;
            int  last_index = 0;
            char **argv     = nullptr;
            for ( int i = 0; i <= length; i++ ) {
                if ( temp[i] == ' ' || temp[i] == '\0' ) {
                    if ( i != last_index ) {
                        argv       = (char * *) realloc ( argv, sizeof ( char * ) * ( argc + 1 ) );
                        argv[argc] = &temp[last_index];
                        temp[i]    = '\0';
                        argc++;
                    }
                    last_index = i + 1;
                }
            }

            // Run parser.
            this->cmd_parser ( argc, argv );

            // Free
            free ( argv );
            free ( temp );
        } while ( true );
    }

    void cmd_parser ( int argc, char **argv )
    {
        int index = 0;
        while ( index < argc ) {
            if ( strcmp ( argv[index], "view" ) == 0 ) {
                index++;
                index += this->command_view ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "cat" ) == 0 ) {
                index++;
                index += this->command_cat ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "list" ) == 0 ) {
                index++;
                index += this->command_list ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "edit" ) == 0 ) {
                index++;
                index += this->command_edit ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "add" ) == 0 ) {
                index++;
                index += this->command_add ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "move" ) == 0 ) {
                index++;
                index += this->command_move ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "delete" ) == 0 ) {
                index++;
                index += this->command_delete ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "export" ) == 0 ) {
                index++;
                index += this->command_export ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "import" ) == 0 ) {
                index++;
                index += this->command_import ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "projects" ) == 0 ) {
                index++;
                index += this->command_projects ( argc - index, &argv[index] );
            }
            else if ( strcmp ( argv[index], "archive" ) == 0  ) {
                index++;
                index += this->command_archive ( argc - index, &argv[index] );
            }
            else if  ( std::string ( argv[index] ) == "help" ) {
                index++;
                notes_print_warning ( "<Once upon a time there will be a help message here.>\n" );
            }
            else if ( strcmp ( argv[index], "gc" ) == 0 ) {
                index++;
                storage->gc ( this->notes );
            }
            else if ( strcmp ( argv[index], "search" ) == 0 ) {
                index++;
                if ( index < argc ) {
                    NotesFilter filter ( this->notes );
                    // Ignore archived notes.
                    if ( show_archive ) {
                        filter.filter_not_archive ();
                    }
                    else {
                        filter.filter_archive ();
                    }
                    try {
                        // Create Query:
                        std::string query = ".*";
                        query += argv[index];
                        query += ".*";

                        std::regex          rm ( query, std::regex_constants::icase | std::regex_constants::optimize );
                        index++;
                        std::vector <Note*> results;
                        for ( auto &note : filter.get_filtered_notes () ) {
                            if ( note->search (  rm ) ) {
                                results.push_back ( note );
                            }
                        }
                        this->display_notes ( results );
                    } catch ( ... ) {
                        std::cerr << "Failed to create regex" << std::endl;
                    }
                }
            }
            else {
                notes_print_error ( "Invalid command: '%s'\n", argv[index] );
                return;
            }
        }
        if ( argc == 0 ) {
            this->command_list ( 0, NULL );
        }
    }

    static void sigchld ( int i )
    {
        wait ( 0 );
    }

    // Merge this with the other file open.
    FILE *sopen ()
    {
        int   fds[2];
        pid_t pid;

        if ( socketpair ( AF_UNIX, SOCK_STREAM, 0, fds ) < 0 ) {
            return NULL;
        }

        signal ( SIGCHLD, sigchld );

        switch ( pid = vfork () )
        {
        case -1:            /* Error */
            close ( fds[0] );
            close ( fds[1] );
            return NULL;
        case 0:             /* child */
            close ( fds[0] );
            //        dup2 ( fds[1], 0 );
            close ( STDIN_FILENO );
            close ( STDERR_FILENO );
            dup2 ( fds[1], 1 );
            close ( fds[1] );
            execlp ( "git", "git", "-C", this->get_path ().c_str (),
                     "ls-tree", "-r", "--name-only", "--full-name", "HEAD", NULL );
            _exit ( 127 );
        }
        /* parent */
        close ( fds[1] );
        return fdopen ( fds[0], "r+" );
    }




    void Load ( )
    {
        // Iterate over all files in the git index.
        FILE *f = sopen ();
        if ( f == nullptr ) {
            notes_print_error ( "Failed to list files\n" );
        }
        size_t buffer_length = 0;
        char   *buffer       = nullptr;
        while ( getline ( &buffer, &buffer_length, f ) >= 0 ) {
            if ( buffer[strlen ( buffer ) - 1] == '\n' ) {
                buffer[strlen ( buffer ) - 1] = '\0';
            }
            // Find filename.
            char *filename = nullptr;
            for ( int iter = strlen ( buffer ) - 1; iter >= 0 && buffer[iter] != '/'; iter-- ) {
                filename = &buffer[iter];
            }

            // Find project.
            Project *p = this;
            if ( filename != NULL && filename != buffer ) {
                *( filename - 1 ) = '\0';
                for ( unsigned int iter = 0; iter < strlen ( buffer ); iter++ ) {
                    if ( buffer[iter] == '/' ) {
                        buffer[iter] = '.';
                    }
                }
                p = this->get_or_create_project_from_name ( buffer );
            }

            Note *note = new Note ( p, &settings, filename );
            // Add to the flat list in the main.
            this->notes.push_back ( note );
        }
        if ( buffer != nullptr ) {
            free ( buffer );
        }
        fclose ( f );
        // Gives them UIDs.
        for ( auto &note : this->notes ) {
            note->set_id ( storage->get_id ( note->get_relative_path () ) );
        }

        sort_notes ();
    }


    void sort_notes ()
    {
        // Sort the notes.
        std::sort ( this->notes.begin (), this->notes.end (), notes_print_sort );
    }
};


int main ( int argc, char ** argv )
{
    INIT_TIC_TAC ()

    NotesCC * notes = new NotesCC ( );

    // Parse some options we might want to check before opening the db.
    notes->pre_parse_settings ( argc, argv );

    // Open repository
    if ( notes->open_repository ( ) ) {
        notes->run ( argc, argv );
    }

    delete notes;

    TIC ( "Total runtime: " );
    return EXIT_SUCCESS;
}
