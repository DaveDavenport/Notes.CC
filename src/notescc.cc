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
/**
 * This project is written in C++, but tries to stick closer to C.
 * Classes, list, strings are ok.. Templates are already doubtful.
 */

/**
 * Class representing a note.
 *
 * Todo how to get a consistent ID?
 * Sort list by ... then assign?
 */
class Note
{
    private:
        Project *project = nullptr;
        std::string filename;

        // Note properties
        std::string title;
        std::string last_edit;
        unsigned int id;

    public:
        Note(Project *project, const char *filename);
};



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

/**
 * Notes implementation code.
 * TODO: Move to own file.
 */
Note::Note(Project *project, const char *filename):
    project(project), filename(filename)
{
    std::string fpath = project->get_path()+"/"+filename;
    std::cout << "Path: " << fpath << std::endl;

    FILE *fp = fopen(fpath.c_str(), "r");
    assert(fp != nullptr);
    char buffer[1024];
    int start = 0;
    while(fgets(buffer, 1024, fp) != NULL && start < 2)
    {
        if(buffer[0] == '-') {
            start++;
            continue;
        }
        char *sep = strstr(buffer, ":");
        if(sep != NULL) {
            *sep = '\0';
            if(strcasecmp(buffer, "title") == 0) {
                this->title = (sep+1);
                // trim trailing \n
                this->title.erase(
                        this->title.end()-1,
                        this->title.end());

                std::cout << "title: " << this->title << std::endl;
            }
        }
    }
    fclose(fp);
}

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
