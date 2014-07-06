#ifndef __PROJECT_H__
#define __PROJECT_H__

// Forward
class Note;

/**
 * This class represents a Project.
 *
 * A project on it own can contain sub-projects and notes.
 */
class Project
{
private:
// Pointer to the parent Project.
    Project             *parent = nullptr;
// Name of this project.
    std::string         name;
// List of notes contained in this project. (project does not own note)
    std::list<Note *>   notes;
// List of children projects (projects owns it and should free it)
    std::list<Project*> child_projects;

protected:
    void set_parent ( Project *parent );
    bool is_root ();

public:
    Project( const char *name );

    virtual ~Project();

    std::string get_name ();

/**
 * Add hierarchical structure.
 */
    void add_subproject ( Project *child );

    void add_note ( Note *note );

    void print ();

    virtual std::string get_path ();

    void list_projects ();

    unsigned int get_num_notes ()
    {
        return this->notes.size ();
    }
    unsigned int get_num_notes_recursive ()
    {
        unsigned int value = this->notes.size ();
        for ( auto p : child_projects ) {
            value += p->get_num_notes_recursive ();
        }
        return value;
    }

    const std::list < Project *> &get_child_projects () const
    {
        return this->child_projects;
    }
};

#endif
