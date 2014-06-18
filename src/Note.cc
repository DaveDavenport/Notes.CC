#include <iostream>
#include <algorithm>
#include <assert.h>

#include <cstring>
#include <list>

#include <Project.h>
#include <Note.h>

/**
 * Notes implementation code.
 */
Note::Note(Project *project, const char *filename):
    project(project), filename(filename)
{
    std::string fpath = project->get_path()+"/"+filename;

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
