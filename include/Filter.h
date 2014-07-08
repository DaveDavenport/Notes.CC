#ifndef __FILTER_H__
#define __FILTER_H__
class NotesFilter
{
private:
    std::vector<Note *> start_notes;
public:
    NotesFilter( std::vector< Note *> notes )
    {
        // Copy the list!
        start_notes = notes;
    }

    void add_filter ( std::string value );

    const std::vector<Note *> &get_filtered_notes () const
    {
        return start_notes;
    }
};
#endif // __FILTER_H__
