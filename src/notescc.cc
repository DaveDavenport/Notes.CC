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

/**
 * This project is written in C++, but tries to stick closer to C.
 * Classes, list, strings are ok.. Templates are already doubtful.
 */




// The Main object, this is also the root node.
class NotesCC : public Project
{
    private:
        std::string db_path;
        // Root project.
        std::list<Note *> notes;


    public:
       NotesCC(const char *path) : Project("")
        {
            db_path = path;

            this->Load(this,"");
        }
        ~NotesCC()
        {
            for ( auto note : notes )
            {
                delete note;
            }
        }

        void print_projects()
        {
            this->print();
        }

        std::string get_path() { return db_path; }

    private:
        void Load(Project *node, std::string path)
        {
            DIR *dir = opendir((db_path+path).c_str());
            if(dir != NULL)
            {
                struct dirent *dirp;
                while( ( dirp = readdir(dir)) != NULL )
                {
                    // Skip hidden files (for now)
                    if(dirp->d_name[0] == '.') continue;
                    // Project
                    if(dirp->d_type == DT_DIR)
                    {
                        Project *p = new Project(dirp->d_name);
                        node->add_subproject(p);

                        // Recurse down in the structure.
                        std::string np = path+"/"+dirp->d_name;
                        Load(p,np);
                    }
                    // Note
                    else if (dirp->d_type == DT_REG)
                    {
                        Note *note = new Note(node, dirp->d_name);
                        // Add to the flat list in the main.
                        this->notes.push_back(note);
                        node->add_note(note);
                    }
                }
                closedir(dir);
            }
        }
};


int main ( int argc, char ** argv )
{
    char *path = NULL;

    if(asprintf(&path, "%s/Notes2/", getenv("HOME")) == -1) {
        fprintf(stderr, "Failed to get path\n");
        return EXIT_FAILURE;
    }

    NotesCC notes(path);
    notes.print_projects();

    free(path);
    return EXIT_SUCCESS;
}
