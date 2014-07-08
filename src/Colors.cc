#include <stdio.h>
#include <stdarg.h>
#include <Colors.h>

static bool quiet = false;

void notes_print_error ( const char *format, ... )
{
    if ( quiet ) {
        return;
    }
    fputs ( color_red_bold "[ERROR   ]: " color_reset, stderr );
    va_list ap;
    va_start ( ap, format );
    vfprintf ( stderr, format, ap );
    va_end ( ap );
}

void notes_print_info ( const char *format, ... )
{
    if ( quiet ) {
        return;
    }
    fputs ( color_blue_bold "[INFO    ]: " color_reset, stdout );
    va_list ap;
    va_start ( ap, format );
    vfprintf ( stdout, format, ap );
    va_end ( ap );
}
void notes_print_warning ( const char *format, ... )
{
    if ( quiet ) {
        return;
    }
    fputs ( color_yellow_bold "[WARNING ]: " color_reset, stdout );
    va_list ap;
    va_start ( ap, format );
    vfprintf ( stdout, format, ap );
    va_end ( ap );
}

void notes_print_quiet ()
{
    quiet = true;
}
