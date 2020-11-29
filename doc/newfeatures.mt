

			   Griljor Meeting
			Monday May 13th, 1991



        New Features that should be added to Griljor for Version 1.0

Done	Features
====	========

	>> Fix editmap to support infinite objects per square

	>> Add cool sight limited rooms

	>> Make everything into the same type of objects (so, for example,
	   someone could pick up another person)

	>> Give objects the ability to contain other objects

	>> Install FORTH code into Griljor
	   * Write monsters
	   * Add ablilty to override class variables in eash instance

	>> reset stats should have "are you sure?" feature
	   * have "expert mode" option to not have prompts

	>> Properties window
	   * turn debugging on and off (or command line option)
	   * turn expert mode on/off
	   * set fast/slow computer speed option
	   * Save/Load properties (load at startup)
	   * Select diretory locations: maps, objects, player bitmaps (see #3)

	>> Add an instructions feature that allows people to scan the
	   full documentation from the the game select screen.

	>> pathname of the library and names of the library directories
	   should be set by the properties window, so that griljor
	   can be run in a different environment than it was compiled in.

Yes	>> Check the magics code and allow objects to drain magic power
	   points.  This will limit use of powerful objects, and
	   provide for magic points to be used for something, as spells
	   are not going to be implemented in version 1.

	>> Allow editmap to select times of day and darkness.

	>> Allow maps to have an attached help file that people can
	   look at during the game, a theme file.

	>> Packets and other transmissions should contain major and minor
	   version numbers so that different, incompatible versions of griljor
	   can avoid each other.  If major version number doesn't match, there
	   should not be interaction between players.  If minor version numbers
	   don't match, then just a warning message should be given.
	* Key tip from Doug:  use SCCS to automatically update version numbers

	>> There should be a command line option people can use to make
	   a game local, and a compiler option that would make all games local.
	   Games private to local machines would be used for slow nets,
	   testing unfinished maps, testing changes to griljor etc.
