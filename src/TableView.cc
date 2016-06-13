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
#include <string>
#include <TableView.h>

#include <Colors.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

TableColumn::TableColumn()
{
}


/**
 * TableColumn
 */
void TableColumn::set_color ( const char *color )
{
    this->color = color;
}

void TableColumn::set_header ( const std::string &name )
{
    this->column_name = name;
    if ( column_name.length () > this->width ) {
        this->width = column_name.length ();
    }
}

void TableColumn::set_value ( unsigned int row, std::string value )
{
    if ( row >= fields.size () ) {
        fields.resize ( row + 1 );
    }
    fields[row] = value;
    if ( value.size () > width ) {
        width = value.size ();
    }
}


void TableColumn::print ( unsigned int row ) const
{
    if ( color ) {
        fputs ( this->color, stdout );
    }
    printf ( this->format, this->width,
             ( fields[row].substr ( 0, this->width ) ).c_str () );
    if ( color ) {
        fputs ( color_reset, stdout );
    }
}

const std::string & TableColumn::get_header () const
{
    return this->column_name;
}

unsigned int TableColumn::get_width () const
{
    return this->width;
}
void TableColumn::set_width ( unsigned int width )
{
    this->width = width;
}

/**
 * TableView
 */

TableView::TableView ()
{
    // Get size of terminal.

    struct winsize w;
    ioctl ( STDOUT_FILENO, TIOCGWINSZ, &w );

    this->terminal_width  = w.ws_col;
    this->terminal_height = w.ws_row;
}

void TableView::add_column ( std::string name, const char *color )
{
    unsigned int index = columns.size ();
    columns.resize ( index + 1 );
    columns[index].set_header ( name );
    columns[index].set_color ( color );
}

void TableView::print ()
{
    unsigned int width = 2;
    fputc ( '\n', stdout );
    // Print headers.
    for ( auto &col : columns ) {
        int remainder = this->terminal_width - width;
        // If space is left, print header.
        if ( remainder > 0 ) {
            if ( remainder < col.get_width () ) {
                col.set_width ( remainder );
            }
            width += col.get_width () + 2;

            printf ( "%s%s%-*s%s",
                     color_bold,
                     color_underline,
                     col.get_width (),
                     col.get_header ().substr ( 0, col.get_width () ).c_str (),
                     color_reset );
            fputs ( "  ", stdout );
        }
        else {
            // Disable column.
            col.set_width ( 0 );
        }
    }
    printf ( "\n" );
    // For each row, print the value.
    for ( unsigned int row = 0; row < this->num_rows; row++ ) {
        for ( auto &col : columns ) {
            if ( col.get_width () > 0 ) {
                col.print ( row );
                fputs ( "  ", stdout );
            }
        }
        printf ( "\n" );
    }
    fputc ( '\n', stdout );
}
