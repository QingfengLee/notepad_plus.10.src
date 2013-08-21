# Make file for Macocs30 IDE
# Author Don H
.SUFFIXES: .cpp
CC = g++
RC = windres
SRC = ../src

OBJS = winmain.o Macocs30_IDE.o res.o ScintillaEditView.o FileDialog.o TabBar.o DocTabView.o FindReplaceDlg.o \
StatusBar.o ToolBar.o SysMsg.o StaticDialog.o Process.o Printer.o M30Setting.o M30ProjectFile.o ImageListSet.o

PROGDEMO = ../bin/PowerEditor.exe

INCLUDEFLAGS = -I../../scintilla/include -ISRC/WinControls -ISRC/WinControls/ImageListSet -ISRC/WinControls/StatusBar \
-ISRC/MISC/FileNamStringSpliter -ISRC/MISC/SysMsg -ISRC/MISC/Process -ISRC/ScitillaComponent -ISRC/WinControls/TabBar \
-ISRC/WinControls/ToolBar -ISRC/WinControls/StaticDialog -ISRC/WinControls/OpenSaveFileDialog

#DEFINEFLAGS = -D_WIN32_IE=0x0600

LDFLAGS = -mwindows -Wl,-s -lcomctl32 -lshlwapi -O2

$(PROGDEMO):	$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
#$(OBJS):
#	$(CC) -c  $< -o $@ $^

winmain.o:
	$(CC) $(LDFLAGS) -c $(SRC)/winmain.cpp -o $@ $(INCLUDEFLAGS)


#

res.o: Macocs30_IDE.rc
	$(RC) -o res.o Macocs30_IDE.rc


clean:
	del /q *.exe *.o
