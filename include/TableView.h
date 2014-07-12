/**
 * Notes.CC
 *
 * MIT/X11 License
 * Copyright (c) 2014 Qball  Cow <qball@gmpclient.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef __TABLE_VIEW_H__
#define __TABLE_VIEW_H__

#include <vector>

class TableColumn
{
const char *format_left_align  = "%*s";
const char *format_right_align = "%-*s";
private:
    // Name of the column (For header)
    std::string              column_name;
    // List of fields.
    std::vector<std::string> fields;
    // color of the column.
    const char               *color = nullptr;

    const char               *format = format_right_align;

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

    void set_left_align ()
    {
        this->format = format_left_align;
    }
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
