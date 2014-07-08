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

void notes_error ( const char *format, ... );
#endif
