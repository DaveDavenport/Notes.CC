#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <settings.h>

void Settings::read_config_file()
{
    // Create path
    std::string homedir = getenv ( "HOME" );
    if(homedir.empty()) {
        return;
    }
    std::string path = homedir+dir_separator+".notesccrc";
    std::ifstream infile(path);
    if(infile.is_open())
    {
        std::string line;
        while (std::getline(infile, line))
        {
            if(line.empty() || line[0] == '#') continue;
            auto sep_pos = line.find_first_of ( '=');
            if(sep_pos != std::string::npos)
            {
                auto key = line.substr(0, sep_pos);
                auto value = line.substr(sep_pos+1,std::string::npos);
                if(key == "EDITOR" ){
                    this->editor = value;
                } else if (key == "REPOSITORY" ) {
                    this->repo_path = value;
                } else if (key == "HTML_VIEWER" ) {
                    this->html_viewer = value;
                }
            }
        }
        infile.close();
    }
}


Settings::Settings()
{
    read_config_file();
}

Settings::~Settings()
{
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

const std::string &Settings::get_editor ()
{
    if ( editor.empty () ) {
        if ( getenv ( "EDITOR" ) != nullptr ) {
            editor = getenv ( "EDITOR" );
            if ( !editor.empty () ) {
                return editor;
            }
        }

        // Fallback.
        editor = "vim";
    }

    return editor;
}

const std::string &Settings::get_html_viewer ()
{
    if ( html_viewer.empty() ) {
            // fallback
            html_viewer = "xdg-open";
    }
    return html_viewer;
}
