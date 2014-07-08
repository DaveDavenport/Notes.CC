#include <string>
#include <TableView.h>

#include <Colors.h>

/**
 * TableColumn
 */
void TableColumn::add_entry ( std::string field )
{
    this->fields.push_back ( field );
    if ( field.length () > this->width ) {
        this->width = field.length ();
    }
}
void TableColumn::set_color ( const char *color )
{
    this->color = color;
}

void TableColumn::set_header ( const std::string name )
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
    printf ( this->format, this->width, fields[row].c_str () );
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

/**
 * TableView
 */
void TableView::add_column ( std::string name, const char *color )
{
    unsigned int index = columns.size ();
    columns.resize ( index + 1 );
    columns[index].set_header ( name );
    columns[index].set_color ( color );
}

void TableView::print ()
{
    fputc('\n', stdout);
    // Print headers.
    for ( auto col : columns ) {
        printf ( "%s%s%-*s%s",
                 color_bold,
                 color_underline,
                 col.get_width (),
                 col.get_header ().c_str (),
                 color_reset );
        fputs ( "  ", stdout );
    }
    printf ( "\n" );
    // For each row, print the value.
    for ( unsigned int row = 0; row < this->num_rows; row++ ) {
        for ( auto col : columns ) {
            col.print ( row );
            fputs ( "  ", stdout );
        }
        printf ( "\n" );
    }
    fputc('\n', stdout);
}
