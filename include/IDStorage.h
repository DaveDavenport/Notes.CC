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

#ifndef __ID_STORAGE_H__
#define __ID_STORAGE_H__

// This class should provide stable id's for notes per pc.
// IDs should not be stored in the note itself as it can lead to
// conflicts when editing notes on multiple pc's.
// Path is used as uid.

class IDStorage
{
private:
    std::string                          cache_path;
    std::map <std::string, unsigned int> idmap;
    bool                                 changed = false;

    // Format:
    // <id> <path>
    void read ();

    void write ();

public:
    IDStorage ( const std::string &repo_path );
    ~IDStorage ();


    /**
     * @param path The path to get an id for.
     *
     * Returns an ID for a given note path.
     * Creates a new id when not found.
     */
    unsigned int get_id ( const std::string path );

    /**
     * @param id The id that is moved.
     * @param path_new The new path.
     *
     * Move the id to new path.
     */
    void move_id ( const std::string path_old, const std::string path_new );

    /**
     * @param id The id that is released.
     *
     * Release the id back for re-use.
     */
    void delete_id ( const std::string path_old );

    /**
     * @param notes List of notes.
     *
     * Clears the list and refills it.
     * Helper function.
     * TODO: Would be nice if this class did not need the knowledge about
     * notes;
     */
    void gc ( std::vector<Note *> notes );
};
#endif // __ID_STORAGE_H__
