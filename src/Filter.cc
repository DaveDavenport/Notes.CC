#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Project.h>
#include <Note.h>

#include <Filter.h>

void NotesFilter::add_filter ( std::string value )
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
