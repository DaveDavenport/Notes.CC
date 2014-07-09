#ifndef __SETTINGS_H__
#define __SETTINGS_H__


class Settings {
public:
    static const char dir_separator = '/';
    Settings();
    ~Settings();

    const std::string &get_repository ();
    const std::string &get_editor ();
    const std::string &get_html_viewer ();

private:
    void read_config_file ();

    std::string repo_path;
    std::string editor;
    std::string html_viewer;
};

#endif
