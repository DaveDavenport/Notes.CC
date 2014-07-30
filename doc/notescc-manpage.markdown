# NOTESCC 1 Notes.CC

## NAME

Notes.CC - Command-line Notes application written in C++

## SYNOPSIS

**Notes.CC** command
 
## DESCRIPTION

**Notes.CC** is a simple command-line tool that allows you to quickly take (using your favorite
editor)
notes, categories them in projects, search them and view them. It uses GIT for version management
and synchronization. The tool was originally written in Bash, but displaying the nice table with
notes became pretty slow. Therefor it was rewritten in C++.


## REPOSITORY

**Notes.CC** uses a easy to manipulate folder structure on the filesystem to store and organize the
notes. Each notes are ordered into directories that represent projects (See PROJECTS in this page).
Each note itself is a text file that uses the markdown syntax. The first line of the note is used as
the title. **Notes.CC** appends some extra (optional) info in a header to keep track of modification
time and revisions. In a later version it might use the GIT back end to get this information.

## Getting started

Create a new repository directory:

```
$ mkdir ~/Notes/
$ cd ~/Notes/
$ git init .
```

Create a configuration file: `~/.notesccrc` file.

```
REPOSITORY=/home/<user>/Notes/
OFFLINE=true
```

We set offline mode *true*, because no remote repository is defined.

Now run:

```
$ notescc add
```

to create your first note.



## LICENSE

MIT/X11

## USAGE

The first argument passed to **Notes.CC** is a command. This describes the action you want to take
e.g.  `list` the notes or `edit` it. Each command in its turn takes a certain amount of (optional)
arguments.


Commands interacting with the repository:

 * `list`: Give a list of the notes.

 * `projects`: Give an overview of the projects.

 * `add`: Add a note to the repository.

 * `import`: Import a markdown document into the repository as note. 

The following commands should not be needed when using **Notes.CC** auto pull/push.

 * `push`: If there are changes, push them to the remote server.

 * `pull`: Pull the latest changes from the server.

Commands working on an existing note:

 * `view`

 * `cat`

 * `export`

 * `edit`

 * `delete`

 * `move`

Other commands:

  * `interactive`: Go into an interactive shell.


## COMMANDS

### list

**Usage:** `list` [FILTER]

Lists all the notesi in a nice looking table-view  matching the FILTER (if filter is not set all
notes are listed).



### projects

**Usage:** `projects`


### add

**Usage:** `add` [PROJECT NAME]

Adds a new note to the specified project (it is possible to specify no project). The note is created
and directly opened for editing.


### import

**Usage:** `import` [PATH] [PROJECT NAME]

Imports a markdown document pointed to by [PATH] into the local repository. If a [PROJECT NAME] is
specified, it is imported into that specific project.

### edit 

**Usage:** `edit` [NOTE ID or FILTER]

## NOTE ID

Each note gets an ID, this ID is unique per PC. An ID is released when the note is removed and can
then be re-assigned to a new Note.

These IDs are not synchronized between PC's. This to avoid possible synchronization issues when
multiple notes are created on different PC's and then synced via GIT.

## PROJECT NAME

A project name consists out of a simple ASCII string. Nested projects are separated by a single `.`.
For example Work.ICT  means Project Work has a sub-project ICT. In the file system the Project
define the directory structure used to organize the notes.


## FILTER

The filter in **Notes.CC** tries to be very flexible, you can specify the [NOTE ID] or a specific
keyword. If this fails, the filter will search the title and project of a note for a match for the
query.  The searching is done tokenized.

### Filter keywords

Currently **Notes.CC** supports the following keywords:

* *last*: The last edited note.

## Extra command line flags

* *--offline*: Do not try to pull/push.

* *--repo* [REPO PATH]: Set the repository path to [REPO PATH].

* *--nopull*: Do not pull from the repository, but do push when needed.

    This command is useful if you do not want the pull delay every time you run NotesCC.

## Config File

The config file supports the following settings.

### HTML_VIEWER
```
HTML_VIEWER=xdg-open
```
Set the html viewer to use.

### REPOSITORY
```
REPOSITORY=<path>
```
The repository path. This should be a local checkout of a git repository that contains the notes.

### EDITOR

```
EDITOR=vim
```

The editor should block until it is done. Editors that fork to the background will not work.
E.g. when using `gvim`, use `gvim -f` to make sure it does not fork.

### OFFLINE
```
OFFLINE=true
```
Puts **Notes.CC** in offline mode.

### NOPULL
```
NOPULL=true
```
Tell **Notes.CC** not to do a git pull on each run.

## WEBSITE

* Website can be found at [here](http://sarine.nl/).
* Bugtracker can be found [here](not now).

## Contact

For help using **Notes.CC**, you can get support at 
IRC:  #gmpc at irc.freenode.net


##AUTHOR

**Notes.CC** is developed by Qball Cow <qball@gmpclient.org>

##Bug Hunters

Greg
