#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <basedir.h>

class Settings {
    xdgHandle    xdg_handle;

public:
    static const char dir_separator = '/';
    Settings();
    ~Settings();

    const std::string &get_repository();

private:
    std::string repo_path;
} ;

#endif
