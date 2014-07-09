#ifndef __FILTER_H__
#define __FILTER_H__
#include <vector>
#include <list>
class NotesFilter
{
private:
    std::list<Note *> start_notes;
public:

    NotesFilter( std::vector< Note *> notes );
    void add_filter ( std::string value );

    const std::list<Note *> &get_filtered_notes () const
    {
        return start_notes;
    }
};
#endif // __FILTER_H__
