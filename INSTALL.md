

First Use
---------

Create a repository:

    mkdir ~/Notes/
    cd ~/Notes/
    git init .

Copy the config file '''notesccrc''' to '~/.notesccrc' and edit it for your needs.


Pushing Repository
------------------

On the *push* and *pull* command notescc will execute the following steps:

push():

   connect to 'origin'
   push origin 'refs/heads/<cur branch>'
   pull()

pull():

    fetch from 'origin'
    Check merge state
    if ( ff ) do fast-forward

At the moment only fast forward merges are supported.

NOTE: libgit2 does not support ssh aliases as push targets, so use the following syntax:

    ssh://<user@><host><:port>/<Path>/
