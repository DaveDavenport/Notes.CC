#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <settings.h>

Settings::Settings()
{
    if ( xdgInitHandle ( &xdg_handle ) == NULL ) {
        fprintf ( stderr, "Failed to initialize XDG\n" );
        exit ( EXIT_FAILURE );
    }
}

Settings::~Settings()
{
    xdgWipeHandle ( &xdg_handle );
}

const std::string &Settings::get_repository ()
{
    if ( repo_path.empty () ) {
        if ( getenv ( "NOTES_DIR" ) != nullptr ) {
            repo_path = getenv ( "NOTES_DIR" );
            return repo_path;
        }

        // Fallback.
        std::string homedir = getenv ( "HOME" );
        repo_path = homedir + dir_separator + "Notes2";
    }

    return repo_path;
}
