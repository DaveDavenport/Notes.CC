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

#include <git2.h>

// Readline
#include <readline/readline.h>
#include <readline/history.h>

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
    "move",
    "edit",
    "view",
    "list",
    "delete",
    "projects",
    nullptr
};

git_commit * getLastCommit ( git_repository * repo )
{
    int        rc;
    git_commit * commit = NULL;   /* the result */
    git_oid    oid_parent_commit; /* the SHA1 for last commit */

    /* resolve HEAD into a SHA1 */
    rc = git_reference_name_to_id ( &oid_parent_commit, repo, "HEAD" );
    if ( rc == 0 ) {
        /* get the actual commit structure */
        rc = git_commit_lookup ( &commit, repo, &oid_parent_commit );
        if ( rc == 0 ) {
            return commit;
        }
    }
    return NULL;
}


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

    git_repository      *git_repo        = nullptr;
    git_index           * git_repo_index = nullptr;
    bool                git_changed      = false;


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


    NotesCC()  : Project ( "" )
    {
    }

    void repository_delete_file ( std::string path )
    {
        printf ( "Delete file: %s\n", path.c_str () );

        int rc = git_index_remove_bypath ( git_repo_index, path.c_str () );
        if ( rc != 0 ) {
            fprintf ( stderr, "Failed add changes to index.\n" );
        }
        git_changed = true;
    }
    void repository_stage_file ( std::string path )
    {
        printf ( "Staging file: %s\n", path.c_str () );

        int rc = git_index_add_bypath ( git_repo_index, path.c_str () );
        if ( rc != 0 ) {
            fprintf ( stderr, "Failed add changes to index.\n" );
        }
        git_changed = true;
    }

    bool repository_commit_changes ( )
    {
        git_oid       tree_oid;
        git_oid       oid_commit;
        git_tree      *tree_cmt;
        git_signature *sign;


        if ( git_signature_default ( &sign, git_repo ) == GIT_ENOTFOUND ) {
            git_signature_new ( (git_signature * *) &sign,
                                "NotesCC", "dummy@nothere", 123456789, 0 );
        }


        // Create a tree to commit.
        int rc = git_index_write_tree ( &tree_oid, git_repo_index );
        if ( rc == 0 ) {
            // Lookup the created tree,
            rc = git_tree_lookup ( &tree_cmt, git_repo, &tree_oid );
            if ( rc == 0 ) {
                // Get last commit.
                git_commit       *last_commit = getLastCommit ( git_repo );
                // If no last commit, create root commit.
                int              entries = ( last_commit == nullptr ) ? 0 : 1;
                // Create new commit.
                const git_commit *commits[] = { last_commit };
                git_commit_create ( &oid_commit,
                                    git_repo,
                                    "HEAD",
                                    sign, sign,
                                    NULL,
                                    "Edited note.\n",
                                    tree_cmt, entries, commits );
            }
        }
        git_signature_free ( sign );


        rc = git_index_write ( git_repo_index );
        if ( rc != 0 ) {
            fprintf ( stderr, "Failed to write index to disc.\n" );
            return false;
        }

        return true;
    }

    bool check_repository_state ()
    {
        // Check if it is in a sane state.
        int state = git_repository_state ( git_repo );
        if ( state != GIT_REPOSITORY_STATE_NONE ) {
            fprintf ( stderr, "The repository is not in a clean state.\n" );
            fprintf ( stderr, "Please resolve any outstanding issues first.\n" );
            return false;
        }
        // TODO: Check bare directory
        if ( git_repository_is_bare ( git_repo ) ) {
            fprintf ( stderr, "Bare repositories are not supported.\n" );
            return false;
        }

        // Check open changes.
        git_status_list    *gsl = nullptr;
        git_status_options opts = {
            .version = GIT_STATUS_OPTIONS_VERSION,
            .show    = GIT_STATUS_SHOW_WORKDIR_ONLY,
            .flags   = GIT_STATUS_OPT_INCLUDE_UNTRACKED
        };
        if ( git_status_list_new ( &gsl, git_repo, &opts ) != 0 ) {
            fprintf ( stderr, "Failed to get repository status.\n" );
            return false;
        }

        size_t i, maxi = git_status_list_entrycount ( gsl );
        bool   clean = true;
        for ( i = 0; i < maxi; i++ ) {
            const git_status_entry *s = git_status_byindex ( gsl, i );
            if ( s->status != GIT_STATUS_CURRENT ) {
                clean = false;
            }
        }
        git_status_list_free ( gsl );
        if ( !clean ) {
            fprintf ( stderr, "The git repository is not clean, there are changes in the local \n" );
            fprintf ( stderr, "work directory. Please commit these first.\n" );
            return false;
        }

        if ( git_repository_index ( &git_repo_index, git_repo ) != 0 ) {
            fprintf ( stderr, "Failed to read index from disc.\n" );
            return false;
        }
        return true;
    }
    bool open_repository ( const char *path )
    {
        db_path = path;
        // Check git repository.
        if ( git_repository_open ( &git_repo, path ) != 0 ) {
            fprintf ( stderr, "The repository directory '%s' is not a git repository.\n", path );
            fprintf ( stderr, "Please initialize a new repository using git init.\n" );
            return false;
        }
        if ( !this->check_repository_state () ) {
            return false;
        }

        // Load the notes.
        this->Load ( );

        // Sort the notes.
        std::sort ( this->notes.begin (), this->notes.end (), notes_sort );

        // Gives them UIDs.
        for ( auto note : this->notes ) {
            note->set_id ( ++this->last_note_id );
        }
        return true;
    }


    ~NotesCC()
    {
        if ( git_changed ) {
            printf ( "Commiting changes to git.\n" );
            repository_commit_changes ();
        }

        // Clean git references.
        if ( git_repo_index != nullptr ) {
            git_index_free ( git_repo_index );
        }
        if ( git_repo != nullptr ) {
            git_repository_free ( git_repo );
        }

        // delete notes.
        for ( auto note : notes ) {
            if ( note != nullptr ) {
                delete note;
            }
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

    std::string get_relative_path ()
    {
        return "";
    }

    void display_notes ( const std::vector<Note *> & view_notes )
    {
        TableView view;

        // Add the columns
        view.add_column ( "ID", color_bold );
        view[0].set_left_align ();
        view.add_column ( "Rev.", color_blue );
        view[1].set_left_align ();
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
        if ( nindex < 1 || nindex > (int) notes.size () || notes[nindex - 1] == nullptr ) {
            fprintf ( stderr, "Invalid note id: %d\n", nindex );
            return cargs;
        }
        Note *note = notes[nindex - 1];

        // Edit the note.
        if ( note->edit () ) {
            // Commit the result.
            auto path = note->get_relative_path ();
            repository_stage_file ( path );
        }

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
        if ( nindex < 1 || nindex > (int) notes.size () || notes[nindex - 1] == nullptr ) {
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

    int command_move ( int argc, char **argv )
    {
        int iter = 0;
        if ( argc < 2 ) {
            fprintf ( stderr, "Move requires two arguments: <note id>  <project path>\n" );
            return iter;
        }
        iter++;
        int nindex = -1;
        try {
            nindex = std::stoi ( argv[0] );
        } catch ( ... ) {
        }
        if ( nindex < 1 || nindex > (int) notes.size () || notes[nindex - 1] == nullptr ) {
            fprintf ( stderr, "Invalid note id: %d\n", nindex );
            return iter;
        }
        Note *note = notes[nindex - 1];

        iter++;
        std::string name = argv[1];
        Project     *p   = this->get_or_create_project_from_name ( name );
        if ( p == nullptr ) {
            return iter;
        }
        if ( p == note->get_project () ) {
            printf ( "Destination same as source.\n" );
            return iter;
        }
        std::string old_path = note->get_relative_path ();
        if ( note->move ( p ) ) {
            std::string new_path = note->get_relative_path ();
            this->repository_stage_file ( new_path );
            this->repository_delete_file ( old_path );
        }
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
    int command_projects ( __attribute__( ( unused ) ) int argc, __attribute__( ( unused ) ) char **argv )
    {
        TableView view;
        view.add_column ( "Project", color_blue );
        view.add_column ( "Num. Notes", color_white );
        view[1].set_left_align ();
        unsigned int row = 0;
        for ( auto p : this->get_child_projects () ) {
            command_projects_add_entry ( p, view, row );
        }
        view.print ();
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
                    fprintf ( stderr, "%s is an invalid Project name.\n", pr_name.c_str () );
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

    int command_delete ( int argc, char **argv )
    {
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
        if ( note == nullptr ) {
            fprintf ( stderr, "Note does not exists\n" );
            return cargs;
        }

        // Delete the file from internal structure and working directory.
        if ( note->del () ) {
            // Tell git the file is removed.
            repository_delete_file ( note->get_relative_path () );
            // Delete the entry from the list.
            delete note;
            notes[nindex - 1] = nullptr;
        }
        return cargs;
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
            fprintf ( stderr, "Failed to find or create the project.\n" );
            return retv;
        }

        Note *n = new Note ( p );

        if ( n != nullptr ) {
            this->notes.push_back ( n );
            n->edit ();
            // Commit the result.
            auto path = n->get_relative_path ();
            repository_stage_file ( path );
        }

        return retv;
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
    void interactive ()
    {
        using_history ();
        do {
            char *temp = readline ( "$ " );

            if ( temp == nullptr ) {
                break;
            }
            if ( strcasecmp ( temp, "quit" ) == 0 ) {
                free ( temp );
                break;
            }
            add_history ( temp );
            // Split into arc, argv structure.
            int  length     = strlen ( temp );
            int  argc       = 0;
            int  last_index = 0;
            char **argv     = nullptr;
            for ( int i = 0; i <= length; i++ ) {
                if ( temp[i] == ' ' || temp[i] == '\0' ) {
                    argv       = (char * *) realloc ( argv, sizeof ( char * ) * ( argc + 1 ) );
                    argv[argc] = &temp[last_index];
                    temp[i]    = '\0';
                    argc++;
                    last_index = i + 1;
                }
            }
            this->run ( argc, argv );
            free ( argv );
            free ( temp );
        } while ( true );
    }

    void run ( int argc, char **argv )
    {
        int index = 0;
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
    void Load ( )
    {
        // Iterate over all files in the git index.
        size_t i, maxi = git_index_entrycount ( git_repo_index );
        for ( i = 0; i < maxi; i++ ) {
            const git_index_entry *entry = git_index_get_byindex ( git_repo_index, i );
            // Do some path parsing magick.
            char                  *path = strdup ( entry->path );
            // Find filename.
            char                  *filename = nullptr;
            for ( int iter = strlen ( path ) - 1; iter >= 0 && path[iter] != '/'; iter-- ) {
                filename =
                    &path[iter];
            }
            // Find project.
            Project *p = this;
            if ( filename != path ) {
                *( filename - 1 ) = '\0';
                for ( unsigned int iter = 0; iter < strlen ( path ); iter++ ) {
                    if ( path[iter] == '/' ) {
                        path[iter] = '.';
                    }
                }
                p = this->get_or_create_project_from_name ( path );
            }

            Note *note = new Note ( p, filename );
            // Add to the flat list in the main.
            this->notes.push_back ( note );
            free ( path );
        }
    }
};


int main ( int argc, char ** argv )
{
    char *path = NULL;
    INIT_TIC_TAC ()

    if ( asprintf ( &path, "%s/Notes2/", getenv ( "HOME" ) ) == -1 ) {
        fprintf ( stderr, "Failed to get path\n" );
        return EXIT_FAILURE;
    }
    NotesCC *notes = new NotesCC ();

    if ( notes->open_repository ( path ) ) {
        if ( argc == 2 && strcmp ( argv[1], "interactive" ) == 0 ) {
            notes->interactive ();
        }
        else {
            notes->run ( argc - 1, &argv[1] );
        }
    }

    free ( path );
    delete notes;

    if ( argc < 2 || strcasecmp ( argv[1], "--complete" ) ) {
        TIC ( "finish" );
    }
    return EXIT_SUCCESS;
}
