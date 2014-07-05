#ifndef __TABLE_VIEW_H__
#define __TABLE_VIEW_H__

#include <vector>

class TableColumn
{
private:
    // Name of the column (For header)
    std::string              column_name;
    // List of fields.
    std::vector<std::string> fields;
    // color of the column.
    const char               *color = nullptr;

    // Max width.
    unsigned int width = 0;
public:

    std::string & operator[] ( unsigned int index )
    {
        if ( index >= fields.size () ) {
            fields.resize ( index + 1 );
        }
        return fields[index];
    }

    void set_value ( unsigned int row, std::string value );

    TableColumn()
    {
    };

    void set_color ( const char *color );

    void set_header ( const std::string name );

    void add_entry ( std::string field );

    unsigned int get_width () const;

    const std::string &get_header () const;

    void print ( unsigned int row ) const;
};

class TableView
{
private:
    std::vector<TableColumn > columns;
    unsigned int              num_rows = 0;


public:
    // increment the number of rows.
    void operator++ ( int )
    {
        this->num_rows += 1;
    }

    TableColumn & operator[] ( int index )
    {
        return columns[index];
    }

    // Add a column
    void add_column ( std::string name, const char *color = nullptr );


    void print ();
private:
};

#endif // __TABLE_VIEW_H__
