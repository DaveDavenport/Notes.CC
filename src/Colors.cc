#include <stdio.h>
#include <stdarg.h>
#include <Colors.h>


void notes_error ( const char *format, ... )
{
    fputs ( color_red_bold "[ERROR]: " color_reset, stderr );
    va_list ap;
    va_start ( ap, format );
    vfprintf ( stderr, format, ap );
    va_end ( ap );
}

void notes_info ( const char *format, ... )
{
    fputs ( color_blue_bold "[INFO ]:  " color_reset, stdout );
    va_list ap;
    va_start ( ap, format );
    vfprintf ( stdout, format, ap );
    va_end ( ap );
}
