#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>


class Project
{



};

class Notes
{
    private:
        std::string db_path;

    public:
       Notes(const char *path)
        {
            db_path = path;

            this->Spider("");
        }


    private:
        void Spider(std::string path)
        {
            DIR *dir = opendir((db_path+path).c_str());
            if(dir != NULL)
            {
                struct dirent *dirp;
                while( ( dirp = readdir(dir)) != NULL )
                {
                    if(dirp->d_name[0] == '.') continue;
                    if(dirp->d_type == DT_DIR)
                    {
                        printf("Category: %s\n", dirp->d_name);
                    

                    }
                }
            }
            closedir(dir);
        }


};

int main ( int argc, char ** argv )
{
    char *path = NULL;

    if(asprintf(&path, "%s/Notes/", getenv("HOME")) == -1) {
        fprintf(stderr, "Failed to get path\n");
        return EXIT_FAILURE;
    }

    Notes notes(path);


    free(path);
    return EXIT_SUCCESS;
}
