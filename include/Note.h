#ifndef __NOTE_H__
#define __NOTE_H__

// Forward declaration.
class Project;

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

#endif
