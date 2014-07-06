#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


#include <iostream>
#include <algorithm>
#include <assert.h>

#include <cstring>
#include <list>

#include <Project.h>
#include <Note.h>

#include <rhash.h>

extern "C" {
#include <mkdio.h>
}

/**
 * TODO:
 *  * Remove Magick limit of 1024 for lines.
 */

/**
 * Notes implementation code.
 */
Note::Note( Project *project, const char *filename ) :
    project ( project ), filename ( filename )
{
    std::string fpath = project->get_path () + "/" + filename;

    FILE        *fp = fopen ( fpath.c_str (), "r" );
    assert ( fp != nullptr );
    char        buffer[1024];
    int         start = 0;
    while ( start < 2 && fgets ( buffer, 1024, fp ) != nullptr ) {
        // Only parse section between '-'.
        if ( buffer[0] == '-' ) {
            start++;
            continue;
        }
        if ( start < 1 ) {
            continue;
        }

        // <key>: value format
        char *sep = strstr ( buffer, ":" );
        if ( sep != nullptr ) {
            *sep = '\0';
            sep++;

            if ( strcasecmp ( buffer, "revision" ) == 0 ) {
                sep[strlen ( sep )] = '\0';
                this->revision      = std::stoul ( sep + 1 );
            }
            else if ( strcasecmp ( buffer, "date" ) == 0 ) {
                sep[strlen ( sep )] = '\0';
                char *retv = strptime ( sep + 1, "%a %b %d %T %z %Y", &( this->last_edit_time ) );
                if ( retv == nullptr ) {
                    fprintf ( stderr, "Failed to parse date: |%s|\n", sep + 2 );
                }
            }
        }
    }

    long body_poss = ftell ( fp );
    // Read the title (first line after header)
    this->read_title ( fp );
    body_poss = fseek ( fp, body_poss, SEEK_SET );

    // Calculate HASH of note.
    // This is used to see if the note has changed.
    // TODO: This is not needed at startup, if it gets to slow, move it.
#ifndef NO_HASH
    this->hash = this->calculate_crc ( fp );
#endif
    fclose ( fp );

    project->add_note ( this );
}

// TODO Move these to helper file.
static bool file_not_exists ( const std::string &filename )
{
    struct stat status;
    int         retv = stat ( filename.c_str (), &status );
    if ( retv != 0 ) {
        if ( errno == ENOENT ) {
            return true;
        }
    }
    return false;
}

Note::Note ( Project *p ) :
    project ( p )
{
    if ( !p->check_and_create_path ()  ) {
        fprintf ( stderr, "Failed to create Project path.\n" );
        abort ();
    }

    // Create a new unique filename
    // Not perfect, will have todo.
    std::string fn    = "note-" + std::to_string ( time ( NULL ) ) + ".note";
    int         index = 0;
    while ( !file_not_exists ( fn ) ) {
        fn = "note-" + std::to_string ( time ( NULL ) ) + "-" + std::to_string ( index ) + ".note";
        index++;
    }

    this->filename = fn;

    // Update information and write out empty file.
    time_t cur_time = time ( NULL );
    localtime_r ( &cur_time, &( this->last_edit_time ) );

    std::string fpath      = project->get_path () + "/" + filename;
    FILE        *orig_file = fopen ( fpath.c_str (), "w" );
    if ( orig_file == nullptr ) {
        fprintf ( stderr, "Failed to open note for writing: %s\n", strerror ( errno ) );
        abort ();
    }
    this->write_header ( orig_file );
    fprintf ( orig_file, "New Note." );
    fclose ( orig_file );

    project->add_note ( this );
}

void Note::read_title (  FILE *fp )
{
    char buffer[1024];
    // First line is title.
    if ( fgets ( buffer, 1024, fp ) != nullptr ) {
        int length = strlen ( buffer );
        if ( buffer[length - 1] == '\n' ) {
            buffer[length - 1] = '\0';
        }
        this->title = buffer;
    }
}

unsigned int Note::calculate_crc ( FILE *fp )
{
    char         buffer[1024];
    unsigned int retv = 0;
    rhash        ctx  = rhash_init ( RHASH_CRC32 );
    size_t       rsize;
    while ( ( rsize = fread ( buffer, 1, 1024, fp ) ) > 0 ) {
        rhash_update ( ctx, buffer, rsize );
    }
    rhash_final ( ctx, nullptr );

    rhash_print ( (char *) &( retv ), ctx, 0, RHPR_RAW );
    rhash_free ( ctx );
    return retv;
}

std::string Note::get_modtime ()
{
    char buffer[256];
    strftime ( buffer, 256, "%F", &( this->last_edit_time ) );
    return std::string ( buffer );
}
void Note::print ()
{
    std::cout << this->id << " " << this->project->get_name ();
    char buffer[1024];
    strftime ( buffer, 1024, "%F", &( this->last_edit_time ) );
    std::cout << " " << buffer << " ";
    std::cout << this->title << std::endl;
}

void Note::set_id ( unsigned int id )
{
    assert ( this->id == 0 );
    this->id = id;
}

time_t Note::get_time_t ()
{
    return mktime ( &this->last_edit_time );
}

unsigned long Note::get_revision () const
{
    return this->revision;
}

/**
 * TODO: Cleanup
 * Make a separate launcher that can both be blocking and non-blocking.
 */
void catch_exit ( __attribute__( ( unused ) ) int sig )
{
    while ( 0 < waitpid ( -1, NULL, WNOHANG ) ) {
        ;
    }
}
static pid_t exec_cmd ( const char *cmd )
{
    if ( !cmd || !cmd[0] ) {
        return -1;
    }

    signal ( SIGCHLD, catch_exit );
    pid_t pid = fork ();

    if ( !pid ) {
        setsid ();
        execlp ( "/bin/sh", "sh", "-c", cmd, NULL );
        exit ( EXIT_FAILURE );
    }
    return pid;
}

void Note::view ()
{
    std::string fpath = project->get_path () + "/" + filename;
    char        *path;
    if ( asprintf ( &path, "/tmp/notecc-%u.xhtml", this->hash ) <= 0 ) {
        fprintf ( stderr, "Failed to create note tmp path\n" );
        return;
    }

    FILE *fp = fopen ( fpath.c_str (), "r" );
    if ( fp == nullptr ) {
        fprintf ( stderr, "Failed to open note: %s\n", fpath.c_str () );
        free ( path );
        return;
    }

    // Skip header.
    char buffer[1024];
    int  start = 0;
    while ( start < 2 && fgets ( buffer, 1024, fp ) != nullptr ) {
        if ( buffer[0] == '-' ) {
            start++;
        }
    }

    // Parse remainder of document.
    MMIOT *doc = mkd_in ( fp, 0 );
    fclose ( fp );

    fp = fopen ( path, "w" );
    if ( fp == nullptr ) {
        fprintf ( stderr, "Failed to open tmp file: %s %s\n", path, strerror ( errno ) );
        free ( path );
        mkd_cleanup ( doc );
        return;
    }
    // Generate XHTML page.
    mkd_xhtmlpage ( doc, 0, fp );
    fclose ( fp );
    mkd_cleanup ( doc );

    // Fire up browser.
    char *command;
    if ( asprintf ( &command, "xdg-open %s", path ) > 0 ) {
        exec_cmd ( command );
        free ( command );
    }

    free ( path );
}

void Note::write_body ( FILE *fpout )
{
    std::string fpath = project->get_path () + "/" + filename;
    FILE        *fp   = fopen ( fpath.c_str (), "r" );
    if ( fp == nullptr ) {
        fprintf ( stderr, "Failed to open note: %s\n", fpath.c_str () );
        return;
    }

    // Skip header.
    char buffer[1024];
    int  start = 0;
    while ( start < 2 && fgets ( buffer, 1024, fp ) != nullptr ) {
        if ( buffer[0] == '-' ) {
            start++;
        }
    }

    this->copy_till_end_of_file ( fp, fpout );

    fclose ( fp );
}


void Note::write_header ( FILE *fpout )
{
    fprintf ( fpout, "---\n" );
    // Print date.
    char buffer[256];
    strftime ( buffer, 256, "%a %b %d %T %z %Y", &( this->last_edit_time ) );
    fprintf ( fpout, "date: %s\n", buffer );
    if ( revision > 0 ) {
        fprintf ( fpout, "revision: %lu\n", this->revision );
    }
    fprintf ( fpout, "---\n" );
}

void Note::copy_till_end_of_file ( FILE *fp_edited_in, FILE *fpout )
{
    char   buffer[1024];
    size_t rsize;
    while ( ( rsize = fread ( buffer, 1, 1024, fp_edited_in ) ) > 0 ) {
        fwrite ( buffer, 1, rsize, fpout );
    }
}

bool Note::edit ()
{
    bool changed = false;
    // Create temp filename.
    // This is used to store the edited note.
    char *path;
    if ( asprintf ( &path, "/tmp/notecc-%u.md", this->hash ) <= 0 ) {
        fprintf ( stderr, "Failed to create note tmp path\n" );
        return false;
    }

    // Open the temp file and write the body of the note.
    FILE *fp = fopen ( path, "w" );
    if ( fp == nullptr ) {
        fprintf ( stderr, "Failed to open temp path: %s\n", path );
        free ( path );
        return false;
    }

    this->write_body ( fp );
    fclose ( fp );

    // Execute editor
    char *command;
    if ( asprintf ( &command, "${EDITOR} %s", path ) > 0 ) {
        pid_t pid = exec_cmd ( command );
        // Wait till client is done.
        waitpid ( pid, NULL, 0 );
        printf ( "done: %d \n", pid );
        free ( command );
    }

    // Re-open edited note.
    fp = fopen ( path, "r" );
    if ( fp == nullptr ) {
        fprintf ( stderr, "Failed to open temp path: %s\n", path );
        free ( path );
        return false;
    }

    unsigned int new_hash = this->calculate_crc ( fp );

    if ( new_hash != this->hash ) {
        std::string fpath = project->get_path () + "/" + filename;
        printf ( "Note has been changed\n" );
        printf ( "Saving note: %s\n", fpath.c_str () );
        // Increment revision number. (TODO: get from git?)
        this->revision++;
        // Update last edited time.
        time_t cur_time = time ( NULL );
        localtime_r ( &cur_time, &( this->last_edit_time ) );


        FILE *orig_file = fopen ( fpath.c_str (), "w" );
        if ( orig_file == nullptr ) {
            fprintf ( stderr, "Failed to open original note for writing: %s\n", strerror ( errno ) );
            fprintf ( stderr, "Not saving note, you can find edit here: %s.\n", path );
            free ( path );
            fclose ( fp );
            return false;
        }

        // Write the header.
        this->write_header ( orig_file );

        // Rewind.
        fseek ( fp, 0L, SEEK_SET );
        // Write body
        this->copy_till_end_of_file ( fp, orig_file );
        // Rewind.
        // Get title.
        fseek ( fp, 0L, SEEK_SET );
        this->read_title ( fp );

        // Update body hash.
        this->hash = new_hash;

        fclose ( orig_file );

        printf ( "Note successfully edited.\n" );
        changed = true;
    }
    else {
        printf ( "Note unchanged, doing nothing.\n" );
    }

    free ( path );
    fclose ( fp );
    return changed;
}
