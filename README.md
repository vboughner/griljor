Fun and easy instructions for installing Griljor.  This should work for a web
account (Sun 3/50) without too much trouble.  Some time in the near future,
a distribution version that is easier to install on the various kinds of
machines will be made available.

1. In your home directory, untar the griljor tarfile with:
   'tar xvf griljor.tar'.  This will create directories in your account
   called C/griljor and games/lib/griljor

2. Type 'pwd' to take a look at the absolute path of your account's
    home directory and replace all occurances of
    '/net/water/c60c-3/fa91/labc-4lc' in the C/griljor/config.h
    to whatever your home path is.  You'll have to use 'sccs edit config.h'
    in the C/griljor directory in order to edit config.h

3. Use 'sccs edit Makefile' in the C/griljor directory and edit the
   top few lines of the Makefile (most notably changing occurances of
   'rgcc' to 'gcc'.

4. In the C/griljor directory type 'make two' and pray.

5. Decrease the size of the griljor and grildriver executables
   with 'strip griljor' and 'strip grildriver'.

6. Move griljor and grildriver to the ~/games directory.

7. If you want the map editor or object definition editor, you
   can type 'make editmap' or 'make obtor' respectively.

If you have any questions, please contact vanb@soda.berkeley.edu

Please do not redistribute Griljor source at this time.  The version
you are installing isn't even considered a beta-test version.
