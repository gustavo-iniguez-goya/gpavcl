# gpavcl
A GUI for the pavcl AV command line: https://help.ubuntu.com/community/PandaAntivirus

Probably it won't work on the latest version of pavcl.

Compilation:
``` gcc gpavcl.c -o gpavcl `pkg-config --libs --cflags gtk+-2.0 gthread-2.0` ```
