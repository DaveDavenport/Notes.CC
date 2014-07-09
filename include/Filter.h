#ifndef __FILTER_H__
#define __FILTER_H__
#include <vector>
#include <list>
class NotesFilter
{
private:
    std::list<Note *> start_notes;
public:
    NotesFilter( std::vector< Note *> notes )
    {
        // Copy the list!
        start_notes = std::list<Note *>(notes.begin(),notes.end());
    }

    void add_filter ( std::string value );

    const std::list<Note *> &get_filtered_notes () const
    {
        return start_notes;
    }

    unsigned int count ()
    {
       return start_notes.size();
    }
};
#endif // __FILTER_H__
