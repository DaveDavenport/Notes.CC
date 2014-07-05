#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

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
    while ( fgets ( buffer, 1024, fp ) != nullptr && start < 2 ) {
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
            if ( strcasecmp ( buffer, "title" ) == 0 ) {
                this->title = ( sep + 1 );
                // trim trailing \n
                // TODO: Trim white-space
                this->title.erase (
                    this->title.end () - 1,
                    this->title.end () );
            }
            else if ( strcasecmp ( buffer, "revision" ) == 0 ) {
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

    // Calculate HASH of note.
    // This is used to see if the note has changed.
    // TODO: This is not needed at startup, if it gets to slow, move it.
#ifndef NO_HASH
    rhash  ctx = rhash_init ( RHASH_CRC32 );
    size_t rsize;
    while ( ( rsize = fread ( buffer, 1, 1024, fp ) ) > 0 ) {
        rhash_update ( ctx, buffer, rsize );
    }
    rhash_final ( ctx, nullptr );

    rhash_print ( (char *) &( this->hash ), ctx, 0, RHPR_RAW );
    rhash_free ( ctx );
#endif
    fclose ( fp );
}

std::string Note::get_modtime ()
{
    char buffer[1024];
    strftime ( buffer, 1024, "%F", &( this->last_edit_time ) );
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
    while ( fgets ( buffer, 1024, fp ) != nullptr && start < 2 ) {
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
        pid_t pid = exec_cmd ( command );
        waitpid ( pid, NULL, WNOHANG );
        printf ( "done\n" );
        free ( command );
    }

    free ( path );
}
