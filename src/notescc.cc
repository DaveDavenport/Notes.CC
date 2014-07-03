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

// List of supported commands.
typedef enum _MainCommands {
    MC_ADD,
    MC_EDIT,
    MC_VIEW,
    MC_LIST,
    MC_DELETE,
    MC_NUM

} MainCommands;
const char * commands[] =
{
    "add",
    "edit",
    "view",
    "list",
    "delete",
    nullptr
};

/**
 * This project is written in C++, but tries to stick closer to C.
 * Classes, list, strings are ok.. Templates are already doubtful.
 */

// Display helper class.
// Show information nicely formatted and colored.


// The Main object, this is also the root node.
class NotesCC : public Project
{
    private:
        unsigned int last_note_id = 0;
        std::string db_path;
        // Root project.
        std::list<Note *> notes;


    public:
        static bool notes_sort(Note *a, Note *b)
        {
            int time = (a->get_time_t() - b->get_time_t());
            if(time == 0) {
                return a->get_title().compare(b->get_title()) < 0;
                // TODO sort on filename last resort, so we get a stable sort.
            }
            return time < 0;
        }
       NotesCC(const char *path) : Project("")
        {
            db_path = path;

            this->Load(this,"");

            this->notes.sort(notes_sort);
            for ( auto note : this->notes )
            {
                note->set_id(++this->last_note_id);
            }
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


        int autocomplete(int argc, char **argv)
        {
            if(argc == 0) {
                // List commands.
                for ( int i = 0; commands[i] != nullptr; i++) {
                    std::cout << commands[i]  << std::endl;
                }
                return 0;
            }
            this->list_projects();

            return 1;
        }

        void run(int argc, char **argv)
        {
            int index = 1;
            while(index < argc)
            {
                if(strcmp(argv[index], "--complete") == 0 ) {
                    index++;
                    index+= this->autocomplete(argc-index, &argv[index]);
                }
                else if (strcmp(argv[index], "list") == 0) {
                    for( auto note : notes ) {
                        note->print();
                    }
                    index++;
                }
                else {
                    std::cerr << "Invalid argument: "<< argv[index] << std::endl;
                    return;
                }
            }
        }

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

    notes.run(argc, argv);

    free(path);
    return EXIT_SUCCESS;
}
