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

#include <Settings.h>

#include <Colors.h>


// CRC copied from:
/* Copyright (C) 1986 Gary S. Brown. You may use this program, or
   code or tables extracted from it, as desired without restriction.*/
#define UPDC32( octet, crc )    ( crc_32_tab[( ( crc ) \
                                               ^ ( (uint8_t) octet ) ) & 0xff] ^ ( ( crc ) >> 8 ) )


static uint32_t crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
uint32_t updateCRC32 ( unsigned char ch, uint32_t crc )
{
    return UPDC32 ( ch, crc );
}



/**
 * TODO:
 *  * Remove Magick limit of 1024 for lines.
 */

/**
 * Notes implementation code.
 */
Note::Note( Project *project, Settings *settings, const char *filename ) :
    project ( project ), settings ( settings ), filename ( filename )
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

                // Correctly initialize the isdst field to indicate not available.
                this->last_edit_time.tm_isdst = -1;
                char *retv = strptime ( sep + 1, "%a %b %d %T %z %Y", &( this->last_edit_time ) );
                // We seem to loose daylight saving this way. Not good.
                if ( retv == nullptr ) {
                    notes_print_error ( "Failed to parse date: |%s|\n", sep + 2 );
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
    this->hash = this->calculate_crc ( fp );

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

Note::Note ( Project *p, Settings *settings ) :
    project ( p ), settings ( settings )
{
    if ( !p->check_and_create_path ()  ) {
        notes_print_error ( "Failed to create Project path.\n" );
        abort ();
    }

    // Create a new unique filename
    // Not perfect, will have todo.
    std::string fn    = "note-" + std::to_string ( time ( NULL ) ) + ".markdown";
    int         index = 0;
    while ( !file_not_exists ( fn ) ) {
        fn = "note-" + std::to_string ( time ( NULL ) ) + "-" + std::to_string ( index ) + ".markdown";
        index++;
    }

    this->filename = fn;

    // Update information and write out empty file.
    time_t cur_time = time ( NULL );
    localtime_r ( &cur_time, &( this->last_edit_time ) );

    std::string fpath      = project->get_path () + "/" + filename;
    FILE        *orig_file = fopen ( fpath.c_str (), "w" );
    if ( orig_file == nullptr ) {
        notes_print_error ( "Failed to open note for writing: %s\n", strerror ( errno ) );
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
        // Skip starting header indicator and whitespace.
        size_t start = 0;
        while ( buffer[start] == '#' || isspace ( buffer[start] ) ) {
            start++;
        }
        this->title = &buffer[start];
    }
}

unsigned int Note::calculate_crc ( FILE *fp )
{
    char         buffer[1024];
    unsigned int retv = 0xFFFFFFFF;
    size_t       rsize;
    while ( ( rsize = fread ( buffer, 1, 1024, fp ) ) > 0 ) {
        for ( size_t i = 0; i < rsize; i++ ) {
            retv = updateCRC32 ( buffer[i], retv );
        }
    }
    return retv;
}

std::string Note::get_modtime ()
{
    char buffer[256];
    strftime ( buffer, 256, "%F %H:%M", &( this->last_edit_time ) );
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
    MMIOT *doc = get_markdown_doc ();
    if ( doc == nullptr ) {
        return;
    }

    char *path;
    if ( asprintf ( &path, "/tmp/notescc-%u.xhtml", this->hash ) <= 0 ) {
        notes_print_error ( "Failed to create note tmp path\n" );
        return;
    }

    FILE *fp = fopen ( path, "w" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open tmp file: %s %s\n", path, strerror ( errno ) );
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
    if ( asprintf ( &command, "%s %s", settings->get_html_viewer ().c_str (), path ) > 0 ) {
        pid_t pid = exec_cmd ( command );
        // Wait till client is done.
        waitpid ( pid, NULL, 0 );
        free ( command );
    }

    free ( path );
}

bool Note::write_body ( FILE *fpout )
{
    std::string fpath = project->get_path () + "/" + filename;
    FILE        *fp   = fopen ( fpath.c_str (), "r" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open note: %s\n", fpath.c_str () );
        return false;
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
    return true;
}

bool Note::cat ()
{
    notes_print_info ( "Title:     %s\n", this->title.c_str () );
    notes_print_info ( "Revisions: %u\n", this->revision );
    notes_print_info ( "Last edit: %s\n", this->get_modtime ().c_str () );
    return this->write_body ( stdout );
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
    if ( asprintf ( &path, "/tmp/notescc-%u.markdown", this->hash ) <= 0 ) {
        notes_print_error ( "Failed to create note tmp path\n" );
        return false;
    }

    // Open the temp file and write the body of the note.
    FILE *fp = fopen ( path, "w" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open temp path: %s\n", path );
        free ( path );
        return false;
    }

    this->write_body ( fp );
    fclose ( fp );

    // Execute editor
    char *command;
    if ( asprintf ( &command, "%s %s", settings->get_editor ().c_str (), path ) > 0 ) {
        pid_t pid = exec_cmd ( command );
        // Wait till client is done.
        waitpid ( pid, NULL, 0 );
        free ( command );
    }

    // Re-open edited note.
    fp = fopen ( path, "r" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open temp path: %s\n", path );
        free ( path );
        return false;
    }

    unsigned int new_hash = this->calculate_crc ( fp );

    if ( new_hash != this->hash ) {
        std::string fpath = project->get_path () + "/" + filename;
        notes_print_info ( "Note has been changed\n" );
        notes_print_info ( "Saving note: %s\n", fpath.c_str () );
        // Increment revision number. (TODO: get from git?)
        this->revision++;
        // Update last edited time.
        time_t cur_time = time ( NULL );
        localtime_r ( &cur_time, &( this->last_edit_time ) );


        FILE *orig_file = fopen ( fpath.c_str (), "w" );
        if ( orig_file == nullptr ) {
            notes_print_error ( "Failed to open original note for writing: %s\n", strerror ( errno ) );
            notes_print_error ( "Not saving note, you can find edit here: %s.\n", path );
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

        notes_print_info ( "Note successfully edited.\n" );
        changed = true;
    }
    else {
        notes_print_info ( "Note unchanged, doing nothing.\n" );
    }

    free ( path );
    fclose ( fp );
    return changed;
}
// Delete note.
bool Note::del ()
{
    std::string fpath = project->get_path () + "/" + filename;
    if ( remove ( fpath.c_str () ) == 0 ) {
        this->project->remove_note ( this );
        return true;
    }
    notes_print_error ( "Failed to delete note: %s\n", strerror ( errno ) );
    return false;
}

bool Note::move ( Project *p )
{
    if ( !p->check_and_create_path ()  ) {
        notes_print_error ( "Failed to create Project path.\n" );
        return false;
    }
    std::string old_path = project->get_path () + "/" + filename;
    // Remove from old project
    this->project->remove_note ( this );
    // Link to new project.
    this->project = p;
    this->project->add_note ( this );
    std::string new_path = project->get_path () + "/" + filename;
    if ( rename ( old_path.c_str (), new_path.c_str () ) ) {
        notes_print_error ( "Failed to move note: %s to %s\nError: %s\n",
                            old_path.c_str (), new_path.c_str (), strerror ( errno ) );
        return false;
    }
    return true;
}

bool Note::export_to_file_raw ( const std::string path )
{
    FILE *fp = fopen ( path.c_str (), "w" );
    if ( fp == NULL ) {
        notes_print_error ( "Failed to open file for writing: %s\n",
                            strerror ( errno ) );
        return false;
    }
    if ( !this->write_body ( fp ) ) {
        fclose ( fp );
        return false;
    }

    fclose ( fp );
    return true;
}

bool Note::export_to_file_html ( const std::string path )
{
    MMIOT *doc = get_markdown_doc ();
    if ( doc == nullptr ) {
        return false;
    }

    FILE *fp = fopen ( path.c_str (), "w" );
    if ( fp == NULL ) {
        notes_print_error ( "Failed to open file for writing: %s\n",
                            strerror ( errno ) );
        mkd_cleanup ( doc );
        return false;
    }

    // Generate XHTML page.
    mkd_xhtmlpage ( doc, 0, fp );
    fclose ( fp );
    mkd_cleanup ( doc );

    return true;
}


MMIOT *Note::get_markdown_doc ( )
{
    std::string fpath = project->get_path () + "/" + filename;
    FILE        *fp   = fopen ( fpath.c_str (), "r" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open note: %s\n", fpath.c_str () );
        return nullptr;
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
    return doc;
}

bool Note::import ( const std::string path )
{
    std::string fpath   = project->get_path () + "/" + filename;
    bool        changed = false;
    // Re-open edited note.
    FILE        *fp = fopen ( path.c_str (), "r" );
    if ( fp == nullptr ) {
        notes_print_error ( "Failed to open path: %s\n", path.c_str () );
        return false;
    }
    time_t cur_time = time ( NULL );
    localtime_r ( &cur_time, &( this->last_edit_time ) );

    FILE *orig_file = fopen ( fpath.c_str (), "w" );
    if ( orig_file == nullptr ) {
        notes_print_error ( "Failed to open original note for writing: %s\n", strerror ( errno ) );
        notes_print_error ( "Not saving note, you can find edit here: %s.\n", path.c_str () );
        fclose ( fp );
        return false;
    }

    // Write the header.
    this->write_header ( orig_file );

    unsigned int new_hash = this->calculate_crc ( fp );
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

    notes_print_info ( "Note successfully edited.\n" );
    changed = true;
    fclose ( fp );
    return changed;
}
