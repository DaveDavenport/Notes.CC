#include <iostream>
#include <algorithm>
#include <assert.h>

#include <cstring>
#include <list>

#include <Project.h>
#include <Note.h>

#include <rhash.h>

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
    while ( fgets ( buffer, 1024, fp ) != NULL && start < 2 ) {
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
        if ( sep != NULL ) {
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
