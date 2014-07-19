NOTESCC 1 NotesCC
===============

NAME
----

NotesCC - Command-line Notes application

SYNOPSIS
--------

`NotesCC` command
 
DESCRIPTION
-----------

`NotesCC` is a simple command-line tool that allows you to quickly take (using your favorite editor)
notes, categories them in projects, search them and view them. It uses GIT for version management
and synchronization. The tool was originally written in Bash, but displaying the nice table with
notes became pretty slow. Therefor it was rewritten in C++.


REPOSITORY
----------

`NotesCC` uses a easy to manipulate folder structure on the filesystem to store and organize the
notes. Each notes are ordered into directories that represent projects (See PROJECTS in this page).
Each note itself is a text file that uses the markdown syntax. The first line of the note is used as
the title. `NotesCC` appends some extra (optional) info in a header to keep track of modification
time and revisions. In a later version it might use the GIT back end to get this information.


LICENSE
-------

MIT/X11

USAGE
-----

The first argument passed to NotesCC is a command. This describes the action you want to take e.g.
`list` the notes or `edit` it. Each command in its turn takes a certain amount of (optional) arguments.


Commands interacting with the repository:

 * `list`: Give a list of the notes.

 * `projects`: Give an overview of the projects.

 * `add`: Add a note to the repository.

 * `import`: Import a markdown document into the repository as note. 

 * `push`: Push the repository to its remote location.  (git push)

 * `pull`: Update the repository from its remote location. (git pull)

Commands working on an existing note:

 * view

 * cat

 * export

 * edit

 * delete

 * move

Other commands:

  * `interactive`: Go into an interactive shell.


COMMANDS
--------

`list` [FILTER]


`projects`


`add` [Project Name]

Adds a new note to the specified project (it is possible to specify no project). The note is created
and directly opened for editing.

`edit` [NOTE ID or FILTER]



NOTE ID
-------

Each note gets an ID, this ID is unique per PC. An ID is released when the note is removed and can
then be re-assigned to a new Note.

These IDs are not synchronized between PC's. This to avoid possible synchronization issues when
multiple notes are created on different PC's and then synced via GIT.

PROJECT NAME
------------

A project name consists out of a simple ASCII string. Nested projects are separated by a single `.`.
For example Work.ICT  means Project Work has a sub-project ICT. In the file system the Project
define the directory structure used to organize the notes.


FILTER
------

Filter will search the title and project of a note for a match for the string.
The searching is done tokenized.


WEBSITE
-------

`NotesCC` website can be found at [here](http://sarine.nl/)

AUTHOR
------
Qball Cow <qball@gmpclient.org>

