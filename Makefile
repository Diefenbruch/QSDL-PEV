#
# Makefile fuer die PEV-Library
#
#
# Author: Marc Diefenbruch
# $Date: 1998/02/20 18:16:45 $
#
# (C) 1995 Universitaet GH Essen
#
# Dieses Makefile stellt folgende Operationen zur Verfuegung:
#
# -) make [all]     : Library und Testprogramm erzeugen
# -) make lib       : Library erzeugen
# -) make test      : Testprogramm erzeugen (erzeugt auch Library falls noetig)
# -) make release   : Neue Release der PEV fuer Benutzer zugaenglich machen
# -) make install   :  "      "     "   "   "     "         "         "
# -) make install-lib: Neue Version der PEV-Bibiothek zugaenglich machen
# -) make install-includes: Neue Version der PEV-Header_files zugaenglich machen
# -) make checkout  : alle Projektdateien auschecken
# -) make checkout-lock: alle Projektdateien zum Aendern auschecken
# -) make depend    : alle Abhaengigkeiten zwischen Projektdateien feststellen
# -) make clean     : Objektdateien (*.o) und temporaere Dateien entfernen
# -) make veryclean : make clean + rcsclean + alle Postscriptdateien entfernen
# -) make git-pull  : pull sources from github
# -) make git-push  : push sources to github
# -) make git-add   : add changed sources to git staging
# -) make git-commit: commit staged sources to git HEAD
# -) make postscript: Postscriptdateien aus alle *.h Dateien erzeugen
# -) make print     : alle *.h Dateien ausdrucken
# -) make backup    : Backup von allen *.cpp *.h Dateien und dem Makefile
# -) make find-error: Compilierungsfehler anzeigen
# -) make tags      : Source-Browser-Datei erzeugen
#
# Der Aufbau dieses Makefiles gliedert sich wie folgt:
#
#  0. Globale Umschalter (z.Z zwischen GNU- und SUN-Tools)
#  1. Makefiledirektiven
#  2. Pfade und Dateinamen
#  3. Programme, Kommandos, Kommandooptionen
#  4. Compilierungs- und Linkeroptionen
#  5. Quelldateien des Projekts
#  6. Objektdateien des Projekts
#  7. Makefileregeln (Operationen, s.o.)
#

###############################
# Figure out host system type #
###############################

## Don't rely on $HOSTTYPE being set correctly!
## Try to figure it out ourselves:

## Prevent make from interpreting special characters:
SCRIPT := \
	case `uname -s` in\
    	Linux)	case `uname -m` in\
					i?86) echo ix86-linux;;\
					arm*) echo arm-linux;;\
					sparc) echo sparc-linux;;\
					*) echo unknown-linux;;\
				esac;;\
		SunOS)	echo `uname -p`-SunOS;;\
		*) echo unknown-unknown;;\
	esac

## Run script and store result in variable:
QUEST_HOSTTYPE := $(shell $(SCRIPT))


##########################
# 0. Globale Umschalter: #
##########################

ifeq ($(HOSTTYPE), sparc-SunOS)

#TOOLS = SUN
TOOLS = GNU

else

TOOLS = GNU

endif

#PROFILING = yes
PROFILING = no

##########################
# 1. Makefiledirektiven: #
##########################

.SILENT:
                         # alle Make Operationen ohne Ausgaben

.PHONY:	clean all release
                         # Welche Operationen sollen gespraechig sein?

.SUFFIXES: .cpp .h .o
                         # verwendete Dateiendungen 

############################
# 2. Pfade und Dateinamen: #
############################

INCDIR = $(QUEST_INC_DIR)
                         # Verzeichnis der PEV-Includedateien fuer die
                         # Benutzer der PEV (Generator, ...)

X11_INC_DIR = $(QUEST_X11_INC_DIR)
                         # X11-Includeverzeichnis
X11_LIB_DIR = $(QUEST_X11_LIB_DIR)
                         # X11-Bibliotheksverzeichnis

LIBDIR = ../../lib/$(QUEST_HOSTTYPE)
                         # Verzeichnis der PEV-Library fuer die
                         # Benutzer der PEV (Generator, ...)
OBJBASEDIR = obj
                         # Verzeichnis unter dem die Objektdateien aller Hosts
                         # gespeichert werden sollen
OBJDIR = $(OBJBASEDIR)/$(QUEST_HOSTTYPE)
                         # Verzeichnis in dem die Objektdateien gespeichert
                         # werden sollen
PSDIR = ps
                         # Directory in dem die Postscriptversionen der
                         # Dateien gespeichert werden sollen
LOGFILE = $(USER).$(QUEST_HOSTTYPE).make.log
                         # Protokollierungsdatei fuer den make-Vorgang
DEPFILE = .depend.$(QUEST_HOSTTYPE)
                         # Dependency-Datei (automatisch generiert)
OUTPUT = $(OBJDIR)/libPEV.a
                         # Name des erzeugten Programms/Library
BACKUP = pev
                         # Name des Backupfiles (ohne Endungen!)

vpath %.cpp	$(GENDIR)
                         # Suchepfad(e) fuer .cpp Dateien
vpath %.h	$(GENDIR)
                         # Suchepfad(e) fuer .h Dateien
vpath %.o	$(OBJDIR)
                         # Suchepfad(e) fuer .o Dateien

##############################################
# 3. Programme, Kommandos, Kommandooptionen: #
##############################################

SHELL = /bin/sh
                         # Shell
LEX = flex
                         # Kommando zum Starten des lexikalischen Analysators
LEXFLAGS = -i
                         # Flags fuer Lexer-Generator
YACC = bison
                         # Kommando zum Starten des Parser-Generators
YACCFLAGS = -d -v
                         # Flags fuer Parser-Generator
TAR = tar cf
                         # Befehl zum Erzeugen eines Sourcecode-Archievs
UNTAR = tar xf
                         # Befehl zum Expandieren eines Sourcecode-Archievs
TAR_SUFFIX = tar
                         # Endung welche das tar-Kommando verwendet
COMPRESS = gzip -9
                         # Befehl zum Komprimieren von Dateien
UNCOMPRESS = gzip -d
                         # Befehl zum Dekomprimieren von Dateien
COMPRESS_SUFFIX = gz
                         # Endung welche das Komprimierungsprogramm verwendet
EDITOR = nano
                         # Name des verwendeten Editors

GIT = git                # git

TAGS = ctags -t
                         # Programm zum Erzeugen von Sourcebrowsing-Infos
                         # (wo ist eine bestimmte Funktion definiert?, etc)
MAKE_PS = a2ps
                         # Kommando zum Erzeugen von Postscript aus ASCII
MAKE_PS_FLAGS = -o
                         # Option fuer MAKE_PS
PS_SUFFIX = ps

PRINT = lp -c -d $(PRINTER)
                         # Druckkommando

ifeq ($(TOOLS), GNU)     # GNU-Version ?

CC = gcc
                         # Name des C-Compilers
C++ = g++
                         # Name des C++-Compilers

else                     # SUN-Version

CC = cc
                         # Name des C-Compilers
C++ = CC
                         # Name des C++-Compilers
endif

AR = ar
                         # Name des Archivers (zum Erzeugen von Libraries)
ARFLAGS = r
                         # Flags fuer den Archiver
RCSCLEAN = rcsclean
                         # Befehl zum "Aufraeumen" des Projekts (Loeschen
                         # der nicht mit Lock Option "ausgecheckten"
                         # Dateien
RM = /bin/rm -f
                         # Befehl zum Loeschen von Dateien
MV = /bin/mv
                         # Befehl zum Verschieben von Dateien
CP = /bin/cp -p
                         # Befehl zum Kopieren von Dateien
CHMOD = /bin/chmod
                         # Befehl zum Aendern der Dateiattribute
TOUCH = touch
                         # Befehl zum Aktualisieren des Dateidatums
MKDIR = mkdir -p
                         # Befehl zum Erzeugen von Directories
ifeq ($(TOOLS), GNU)

MAKE_DEPEND = $(C++) -M

else

MAKE_DEPEND = $(C++) -xM

endif
                         # Befehl zum Erzeugen der Abhaengigkeiten
STRIP = strip
                         # Befehl zum entfernen von Symbolen aus Binaerfiles
SED = sed
                         # Name des Stream-Editors sed
COFLAGS += -M -q
                         # Flags for checkout program

#########################################
# 4. Compilierungs- und Linkeroptionen: #
#########################################

ifeq ($(TOOLS), GNU)     # GNU-Version ?

#DEFINES = $(QUEST_THREAD_TYPE) -D_SC_TRACING -D_REENTRANT
DEFINES = -D_SC_TRACING -D_REENTRANT
#DEFINES += -DDEBUG -DDMALLOC -DDMALLOC_FUNC_CHECK
                         # Defines fuer die Compiler
INCLUDES = -I. -I$(QUEST_ADDITIONAL_INC_DIR) -I$(INCDIR) -I$(X11_INC_DIR)
                         # Include-Verzeichnisse fuer die Compiler
                         # QUEST_ADDITIONAL_INC_DIR may be used to specify
                         # system specific include pathes.

DEP_INCLUDES = -I$(X11_INC_DIR)
                         # zusaetzliches Includeverzeichnis fuer MAKEDEPEND
CDEBUG = -O6 -pipe
                         # Debug-/Optimierungsoptionen
CFLAGS = $(CDEBUG) -Wall 
                         # Compiler-Flags
TFLAGS = 
                         # Template-Flags
LIBS = -L$(LIBDIR) -lX11
                         # Libraries die zum Projekt gelinkt werden sollen

else                     # Sun-Version !

DEFINES = $(QUEST_THREAD_TYPE)
                         # Defines fuer die Compiler
INCLUDES = -I$(INCDIR) -I$(DEP_INCDIR) -I$(X11_INC_DIR)
                         # Include-Verzeichnisse fuer die Compiler
DEP_INCLUDES = -I$(X11_INC_DIR)
                         # zusaetzliches Includeverzeichnis fuer MAKEDEPEND
CDEBUG = -O4
                         # Debug-/Optimierungsoptionen
CFLAGS = $(CDEBUG)
                         # Compiler-Flags
TFLAGS = 
                         # Template-Flags
LIBS = -L$(LIBDIR) -lX11
                         # Libraries die zum Projekt gelinkt werden sollen

endif

#################################
# 5. Quelldateien des Projekts: #
#################################

PEHDR = PEEventDispatcher.h PESensor.h PESMachine.h PESProcess.h PESActivity.h PESetup.h PEScanner.h
PDHDR = PDDataType.h 
PCHDR = PCUpdater.h PCController.h
PVHDR = PVXEventDispatcher.h PVMapper.h PVDisplay.h PVDataDisplay.h
HEADERS  = $(PEHDR) $(PDHDR) $(PCHDR) $(PVHDR)
SRCS  = $(HEADERS:.h=.cpp) PETemplates.cpp

##################################
# 6. Objektdateien des Projekts: #
##################################

OBJS  = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

######################
# 7. Makefileregeln: #
######################

default: clean-rubbish $(OBJDIR) $(OUTPUT)

$(OUTPUT): $(OBJS)
	@echo Constructing $(OUTPUT) ...
	$(AR) $(ARFLAGS) $(OUTPUT)\
		$(OBJS)\
		2>> $(LOGFILE)

all: $(OUTPUT) test

$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	@echo Compiling $< ...
	$(C++) -c $(CFLAGS) $(TFLAGS) $(DEFINES) $(INCLUDES) $< -o $(OBJDIR)/$(notdir $@) 2>> $(LOGFILE)

$(OBJBASEDIR):
	@if [ ! \( -d $(OBJBASEDIR) \) ]; then \
		echo Creating $(OBJBASEDIR) ...; \
		$(MKDIR) $(OBJBASEDIR); fi

$(OBJDIR): $(OBJBASEDIR)
	@if [ ! \( -d $(OBJDIR) \) ]; then \
		echo Creating $(OBJDIR) ...; \
		$(MKDIR) $(OBJDIR); fi

$(LIBDIR): 
	@if [ ! \( -d $(LIBDIR) \) ]; then \
		echo Creating $(LIBDIR) ...; \
		$(MKDIR) $(LIBDIR); fi

$(PSDIR): 
	@if [ ! \( -d $(PSDIR) \) ]; then \
		echo Creating $(PSDIR) ...; \
		$(MKDIR) $(PSDIR); fi

$(INCDIR): 
	@if [ ! \( -d $(INCDIR) \) ]; then \
		echo Creating $(INCDIR) ...; \
		$(MKDIR) $(INCDIR); fi

$(INCDIR)/PEV: $(INCDIR)
	@if [ ! \( -d $(INCDIR)/PEV \) ]; then \
		echo Creating $(INCDIR)/PEV ...; \
		$(MKDIR) $(INCDIR)/PEV; fi

$(DEPFILE):
	$(TOUCH) $(DEPFILE)

install-lib: $(OUTPUT) $(LIBDIR)
	@echo Deleting old library from $(LIBDIR) ...
	-$(RM) $(LIBDIR)/$(OUTPUT)
	@echo Installing new library in $(LIBDIR) ...
	$(CP)  $(OUTPUT) $(LIBDIR)

install-includes:  $(HEADERS) $(INCDIR)/PEV
	@echo Deleting old include files from $(INCDIR)/PEV ...
	-$(RM) $(INCDIR)/PEV/*.h
	@echo Installing new include files in $(INCDIR)/PEV ...
	for X in $(HEADERS); do \
		$(CP)  $${X} $(INCDIR)/PEV; done

install: install-includes install-lib

release: install

git-pull:
	@echo Pulling sources form github...
	$(GIT) pull

git-add:
	@echo Staging changed local sources...
	$(GIT) add -A

git-commit:
	@echo Committing changed local sources...
	$(GIT) commit

git-push:
	@echo Pushing sources to github...
	$(GIT) push 

postscript: $(HEADERS) $(PSDIR)
	@for X in $(HEADERS); do \
		echo Generating $$X.$(PS_SUFFIX) from $$X ...; \
		$(MAKE_PS) $(MAKE_PS_FLAGS) $(PSDIR)/$$X.$(PS_SUFFIX) $$X ; done

print: postscript
	$(PRINT) $(PSDIR)/*.$(PS_SUFFIX)
	-$(RM) *.$(PS_SUFFIX)

backup: $(SRCS) $(HEADERS)
	-$(RM) $(BACKUP).$(TAR_SUFFIX) $(BACKUP).$(TAR_SUFFIX).$(COMPRESS_SUFFIX)
	$(TAR) $(BACKUP).$(TAR_SUFFIX)\
		$(SRCS) $(HEADERS) Makefile
	$(COMPRESS) $(BACKUP).$(TAR_SUFFIX)
	-$(MV) $(BACKUP).$(TAR_SUFFIX).$(COMPRESS_SUFFIX) $(BACKUP).taz

fe find-error:
	$(EDITOR) -p $(LOGFILE)

tags: $(SRCS)
	-$(TAGS) $(SRCS)

clean-rubbish:
	-$(RM) *~ core* *.bak $(LOGFILE)

clean-objects:
	-$(RM) $(OBJDIR)/*.o $(OUTPUT) *.o

clean-rcs:
	-@$(RCSCLEAN) 2> /dev/null

clean: clean-rubbish clean-objects

veryclean: clean clean-rcs 
	-$(RM) $(PSDIR)/*.$(PS_SUFFIX) *.$(PS_SUFFIX) *.$(TAR_SUFFIX) *.$(COMPRESS_SUFFIX) *.taz *tags 2> /dev/null

checkout:
	-@$(CO) -q $(HEADERS)\
			$(SRCS)

checkout-lock:
	-@$(CO) -l $(HEADERS)\
			$(SRCS)

replace-headers: $(HEADERS)
	for X in $(HEADERS); do \
		$(MV) $$X $$X.old; \
		$(SED) 's/24.09.95/10.02.98/g' $$X.old >> $$X ; done

replace-sources: $(SRCS)
	for X in $(SRCS); do \
		$(MV) $$X $$X.old; \
		$(SED) 's/24.09.95/10.02.98/g' $$X.old >> $$X ; done

depend: $(HEADERS) $(SRCS)
	@echo Building dependency file $(DEPFILE) ...
	$(MAKE_DEPEND) $(DEFINES) $(SRCS) $(INCLUDES) $(DEP_INCLUDES) > $(DEPFILE)
	$(SED) 's/^\(.*\)\.o/\$$(OBJDIR)\/\1\.o/g' $(DEPFILE) > $(DEPFILE).sed
	$(MV) $(DEPFILE).sed $(DEPFILE)


-include $(DEPFILE)
