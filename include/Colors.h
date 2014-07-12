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
#ifndef __COLORS_H__
#define __COLORS_H__

#define  color_reset           "\e[0m"
#define  color_bold            "\e[1m"
#define  color_underline       "\033[4m"
#define  color_black           "\e[0;30m"
#define  color_red             "\e[0;31m"
#define  color_green           "\e[0;32m"
#define  color_yellow          "\e[0;33m"
#define  color_blue            "\e[0;34m"
#define  color_magenta         "\e[0;35m"
#define  color_cyan            "\e[0;36m"
#define  color_white           "\e[0;37m"
#define  color_white_bold      "\e[1;37m"
#define  color_black_bold      "\e[1;30m"
#define  color_red_bold        "\e[1;31m"
#define  color_green_bold      "\e[1;32m"
#define  color_yellow_bold     "\e[1;33m"
#define  color_blue_bold       "\e[1;34m"
#define  color_magenta_bold    "\e[1;35m"
#define  color_cyan_bold       "\e[1;36m"

void notes_print_error   ( const char *format, ... );
void notes_print_info    ( const char *format, ... );
void notes_print_warning ( const char *format, ... );

void notes_print_quiet ();
#endif
