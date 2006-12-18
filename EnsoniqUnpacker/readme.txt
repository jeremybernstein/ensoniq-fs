
   ____                   _        __  __                   __          
  / __/__  ___ ___  ___  (_)__ _  / / / /__  ___  ___ _____/ /_____ ____
 / _// _ \(_-</ _ \/ _ \/ / _ `/ / /_/ / _ \/ _ \/ _ `/ __/  '_/ -_) __/
/___/_//_/___/\___/_//_/_/\_, /  \____/_//_/ .__/\_,_/\__/_/\_\\__/_/   
                           /_/            /_/                           
         EnsoniqUnpacker v1.23 - a TotalCommander packer plugin


1 What's this?
--------------

EnsoniqUnpacker is a packer plugin for the file manager "TotalCommander"
by Christian Ghisler (http://www.ghisler.com). It allows you to explore the
contents of different Ensoniq related disk images (EDE/EDA/EDT, GKH, Mode1-CD
and plain 1:1 images). You can extract any file from a valid image and save
it to an *.EFE file (instruments, banks, songs, effects, sysex, ...) or just
copy the whole content with all subfolders to another location.

You also can extract all samples from an EFE instrument or listen to them
with TotalCommander.

The plugin is read only. You cannot add/remove/modify any file or folder. 
For this you will need to utilize the file system plugin EnsoniqFS.

2 Installation
--------------

If you have installed another version of EnsoniqUnpacker before, please unin-
stall it (see below). Otherwise TotalCommander will not recognize all new 
features of the plugin.

For installation simply open the "EnsoniqUnpacker.zip" inside TotalCommander
by simply pressing <enter> on that file. TotalCommander will ask you whether
to install this plugin and do all necessary steps automatically.

Do the same with "EnsoniqUnpackerEFE.zip".


3 Uninstallation
----------------

Delete the folders "EnsoniqUnpacker" and "EnsoniqUnpackerEFE" in your
TotalCommander plugins folder (normally "TotalCommander/plugins/wcx"). If it
is not possible to delete them, then they are in use by TotalCommander. You
need to restart TotalCommander. As long as you do not open any new archive,
you will be able to delete these folders.

To remove all setup data you further need to: 
- choose "Configuration/Change Settings Files Directly"
- in the file "wincmd.ini" search for "[PackerPlugins]"
- delete every line containing "EnsoniqUnpacker.wcx"
- delete every line containing "EnsoniqUnpackerEFE.wcx"
- save configuration file.


4 How to use
------------

4.1 The image file reader
-------------------------

After you made it through the installation steps, it is really easy to use
the plugin. In TotalCommander navigate to a folder containing either an EDE,
EDA, EDT, GKH or other Ensoniq formatted CD oder disk image.

For EDE/EDA/EDT/GKH just press <enter> on that archive and the content of the
archive will be displayed. If the archive is broken, you will get an error
message.

For other Ensoniq images (CD image or 1:1 disk image) you need to navigate to
that file and press Ctrl+PageDown. The archive will be opened if EnsoniqUn-
packer can handle it. I do not recommend to add those archive types (which
probably are *.ISO, *.BIN or alike) to the associated file types for Ensoniq-
Unpacker, because in most cases these files do not contain Ensoniq data but
other types of PC formatted media. They should be handled by other software.
Instead, if you know that a file contains Ensoniq data, just use this
Ctrl+PageDown shortcut. It will also work with the above mentioned other
file types.

Once you opened an archive, you can navigate through it (if it contains sub-
folders), select files (and folders) as you are used to in TotalCommander, and
copy (extract) your selection to another place.

All files will be saved as *.EFE, which can hold any filetype used by Ensoniq
(instruments, banks, songs, sequences, O.S., effects, sysex and more).

You might wonder why the EFE files have such strange numbers at the end (like
"AS-1 BANK 1.[30].efe" or "STREO VIOLNS.[03].efe"). This is due to the
following facts: Ensoniq can store files with identical name but with 
different type in one directory. If you copy these files to a DOS/Windows
drive, they all would have the same name (which is impossible there). So they
get a type tag and you can copy all files even with identical name to your
DOS/Windows drive. The original filename will be stored into the EFE header.
You do not need to rename the files if you want to use EnsoniqFS to copy
them back to another Ensoniq drive.

Two additional info files will be shown in the root folder of every diskimage.
They contain statistics of the image (such as disk label and disk layout) and
a complete tree view of all files (including names, sizes and types).


4.2 The instrument file reader
------------------------------

The instrument file reader is responsible for opening EFE instruments. If
you press <enter> on an EFE file, TotalCommander will show you all wave-
samples contained in that file. These wavesamples can be copied out of the
instrument in Windows wave format (PCM, 16 bit, mono) or you can quickly
listen to them by pressing F3 to start the built-in TotalCommander file
viewer.

The "info.txt" file contains information on all wavesamples in an opened
instrument: layer names, wavesample names, sample sizes, sample rates and
key mapping.


5 Latest version
----------------

As of the writing of this document the latest version is v1.23. A newer
version may be available via "http://www.thoralt.de/projects/ensoniqunpacker/".


6 License
---------

Everyone is allowed to use, abuse and distribute this software freely except
for commercial use. If you distribute this software, the author has to be
named clearly. This software MUST NOT be sold without written permisson
of the author. The source code may be freely modified, but the original author
must be clearly stated (give credits!). If you modify the source code, please
let me know about it, I possibly want to integrate your changes into the
official version.


7 Disclaimer
------------

This software is provided "as is". Although the author tested the software,
there is no warranty of merchantability and fitness for a particular purpose.
Use the software at your own risk. The author is not responsible for any
damage to your person, your equipment or your data arising out of the use of
this software.


8 History
---------

v1.23
released 06-11-12
- changed the treeview info file structure for better overview
- added detailed file type descriptions to treeview
- put the name of the disk image in the info files
- speed increase
- different naming scheme

v1.22
released 06-10-23
- added extra infofile with whole tree view of disk
- changed info file names to contain the name of the disk image name

v1.21
released 06-10-22
- fixed some bugs in "info.txt"/key mapping display
- made "info.txt" more resistant against malformed wavesamples

v1.2
released 06-10-21
- added lots of wavesample properties to "info.txt" inside of instrument files
- changed wavesample names inside of instrument files
- EFE instrument detection is now somewhat weaker (detects more files
  correctly)
- fixed: closed all files in case of an error
- included autoinstall from ZIP function

v1.1
released 06-10-15
- added support for ensoniq instrument wavesample browsing

v1.0
released 06-10-11
- reads & autodetects EDE/EDA/EDT, GKH, Mode1-CDROMS, plain images
- no writing capabilities yet

9 Contact
---------

You can drop me an email: thoralt@thoralt.de

10 Greetings
------------

Special thanks go to Jan Petersson for enthusiastic thoroughly testing the
plugin.
