#///////////////////////////////////////////////////////////////////////////
# Makefile
#///////////////////////////////////////////////////////////////////////////
# Copyright (c) 2000 John Adcock.  All rights reserved.
#///////////////////////////////////////////////////////////////////////////
#
#	This file is subject to the terms of the GNU General Public License as
#	published by the Free Software Foundation.  A copy of this license is
#	included with this software distribution in the file COPYING.  If you
#	do not have a copy, you may obtain a copy by writing to the Free
#	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#
#	This software is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details
#///////////////////////////////////////////////////////////////////////////
#
# This software was based on hwiodrv from the FreeTV project Those portions are
# Copyright (C) Mathias Ellinger
#
#///////////////////////////////////////////////////////////////////////////
# Change Log
#
# Date          Developer             Changes
#
# 15 Oct 1998   Mathias Ellinger      initial version
#
# 24 Jul 2000   John Adcock           Original dTV Release
#                                     Updated to make with win98 ddk
#
#///////////////////////////////////////////////////////////////////////////

BUILD_TYPE=base
!INCLUDE $(BASEDIR)\inc\master.mk

!ifdef DEBUG
CVXDFLAGS  = -Zp -Gs -c -DIS_32 -Zi -DDEBLEVEL=1 -DDEBUG -I$(INC32) -I..\common -I..\include -D_VXD_ -DWIN95 -DWIN40COMPAT
LFLAGS     = -machine:i386 -debugtype:cv -debug -VXD
AFLAGS     = -coff -DBLD_COFF -DIS_32 -W2 -Zi -c -Cx -DMASM6 -DDEBLEVEL=1 -DDEBUG -I$(INC32)
TMPDIR=Debug
!else
CVXDFLAGS  = -Zp -Gs -c -DIS_32 -I$(INC32) -I..\common -I..\include -D_VXD_ -DWIN95 -DWIN40COMPAT
LFLAGS     = -machine:i386 -VXD
AFLAGS     = -coff -DBLD_COFF -DIS_32 -W2 -c -Cx -DMASM6 -I$(INC32)
TMPDIR=$(TMPDIR)
!endif


all: ..\..\dTV\dTVdrv95.vxd

$(TMPDIR)\dTVdrv95.obj: dTVdrv95.cpp
        cl $(CVXDFLAGS) -Fo$@ -Fd$(TMPDIR)\vc60.pdb %s

$(TMPDIR)\crtl.obj: crtl.cpp
        cl $(CVXDFLAGS) -Fo$@ -Fd$(TMPDIR)\vc60.pdb %s 

$(TMPDIR)\debugout.obj: ..\common\debugout.cpp
        cl $(CVXDFLAGS) -Fo$@ -Fd$(TMPDIR)\vc60.pdb %s

$(TMPDIR)\ioclass.obj: ..\common\ioclass.cpp
        cl $(CVXDFLAGS) -Fo$@ -Fd$(TMPDIR)\vc60.pdb %s

$(TMPDIR)\pcienum.obj: ..\common\pcienum.cpp
        cl $(CVXDFLAGS) -Fo$@ -Fd$(TMPDIR)\vc60.pdb %s

$(TMPDIR)\vxdstub.obj: vxdstub.asm
        ml $(AFLAGS)  -Fo$@ %s

..\..\dTV\dTVdrv95.vxd: $(TMPDIR)\dTVdrv95.obj $(TMPDIR)\crtl.obj \
		$(TMPDIR)\vxdstub.obj dTVdrv95.def\
		$(TMPDIR)\debugout.obj $(TMPDIR)\ioclass.obj\
		$(TMPDIR)\pcienum.obj
        echo >NUL @<<dTVdrv95.crf
-ignore:4078 -ignore:4039
-DEF:dTVdrv95.def -OUT:..\..\dTV\dTVdrv95.vxd -MAP:..\..\dTV\dTVdrv95.map -LIBPATH:$(LIB32)
$(LFLAGS)
$(BASEDIR)\lib\i386\free\vxdwraps.clb $(TMPDIR)\dTVdrv95.obj 
$(TMPDIR)\crtl.obj $(TMPDIR)\vxdstub.obj $(TMPDIR)\pcienum.obj
$(TMPDIR)\debugout.obj $(TMPDIR)\ioclass.obj
<<
        link @dTVdrv95.crf
        cd ..\..\dTV
        mapsym dTVdrv95
        cd ..\Driver\vxd

clean:
        -@if exist $(TMPDIR)\*.obj del $(TMPDIR)\*.obj
        -@if exist $(TMPDIR)\*.pdb del $(TMPDIR).pdb
        -@if exist ..\..\dTV\dTVdrv95.* del ..\..\dTV\dTVdrv95.*
        -@if exist $(TMPDIR)\*.exp del $(TMPDIR)\*.exp
        
