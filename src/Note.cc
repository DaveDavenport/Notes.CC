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
        // Only parse section between '-'.
        if(buffer[0] == '-') {
            start++;
            continue;
        }
        if(start < 1) continue;

        // <key>: value format
        char *sep = strstr(buffer, ":");
        if(sep != NULL) {
            *sep = '\0';
            if(strcasecmp(buffer, "title") == 0) {
                this->title = (sep+1);
                // trim trailing \n
                // TODO: Trim white-space
                this->title.erase(
                        this->title.end()-1,
                        this->title.end());
            } else if (strcasecmp(buffer, "revision") == 0) {

            } else if (strcasecmp(buffer, "date") == 0 ) {

            }
        }
    }
    fclose(fp);
}

void Note::print()
{
    std::cout << this->id << " " << this->project->get_name() << this->title << std::endl;

}
