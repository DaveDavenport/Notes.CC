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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <fstream>
#include <sstream>
#include <Settings.h>

void Settings::read_config_file ()
{
    // Create path
    std::string homedir = getenv ( "HOME" );
    if ( homedir.empty () ) {
        return;
    }
    std::string   path = homedir + dir_separator + ".notesccrc";
    std::ifstream infile ( path );
    if ( infile.is_open () ) {
        std::string line;
        while ( std::getline ( infile, line ) ) {
            if ( line.empty () || line[0] == '#' ) {
                continue;
            }
            auto sep_pos = line.find_first_of ( '=' );
            if ( sep_pos != std::string::npos ) {
                auto key   = line.substr ( 0, sep_pos );
                auto value = line.substr ( sep_pos + 1, std::string::npos );
                if ( key == "EDITOR" ) {
                    this->editor = value;
                }
                else if ( key == "REPOSITORY" ) {
                    this->repo_path = value;
                }
                else if ( key == "HTML_VIEWER" ) {
                    this->html_viewer = value;
                }
                else if ( key == "OFFLINE" ) {
                    this->offline = (strcasecmp(value.c_str(), "true") == 0);
                }
            }
        }
        infile.close ();
    }
    if(getenv("OFFLINE")) {
        this->offline = strcasecmp(getenv("OFFLINE"), "true") == 0;
    }
}


Settings::Settings()
{
    read_config_file ();
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
        repo_path = homedir + dir_separator + "Notes";
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
    if ( html_viewer.empty () ) {
        // fallback
        html_viewer = "xdg-open";
    }
    return html_viewer;
}

const bool Settings::get_offline()
{
    return this->offline;
}

void Settings::set_offline(bool offline)
{
    this->offline = offline;
}

void Settings::set_repository(std::string &repo_path)
{
    this->repo_path = repo_path;
}

