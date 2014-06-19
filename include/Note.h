#ifndef __NOTE_H__
#define __NOTE_H__

// Forward declaration.
class Project;

/**
 * Class representing a note.
 *
 * TODO how to get a consistent ID?
 * Sort list by ... then assign?
 *
 * TODO: Calculate a CRC off the content of the note?
 */
class Note
{
    private:
        Project *project = nullptr;
        std::string filename;

        // Note properties
        std::string title;
        std::string last_edit;
        unsigned int id = 0;

    public:
        Note(Project *project, const char *filename);

        void print();

        // TODO
        unsigned int calculate_body_crc(){return 0;}
};

#endif
