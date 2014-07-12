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
