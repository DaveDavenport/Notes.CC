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

    unsigned int count ()
    {
        unsigned int retv = 0;
        for ( auto n : start_notes ) {
            if ( n != nullptr ) {
                retv++;
            }
        }
        return retv;
    }
};
#endif // __FILTER_H__
