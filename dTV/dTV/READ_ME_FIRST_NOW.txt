Updated Instructions For New & Existing Developers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Great news!  No batch files or makefile necessary for dTVdrv95 
needed anymore, as I have converted the dTVdrv95 project into 
a true Visual Studio Workspace now just like the other components.
That means no more hardcoded paths.   All you need is your
Visual Studio and a couple of easy-to-install DDK's below.

For other documentation, including HTML documentation, 
please see the "Docs" folder.

Also, to make it easier to quickly package binary distributions, 
the binaries are output to completely separate ..\Debug and ..\Release 
subdirectories right alongside the "Drivers" and "Dtv" subdirectories.
That way, it is easier to zip up and redistribute the binaries
without accidentally zipping up source code or object files as well.
The HTML files have been moved to the "Release" directory too.

Step by step instructions:

1. Your dTV source code can now be in any subdirectory on any drive

2. Preferred development platform is Windows 2000 with Visual Studio 6. 

3. Upgrade your Visual Studio with the latest service pack from:
   http://msdn.microsoft.com/vstudio/

4. Get the Windows 2000 DDK from:
   http://www.microsoft.com/ddk/W2kDDK.htm
   Install as D:\NTDDK or another drive of your choice.
   We'll use drive D: as an example here.

5. Get the Windows 98 DDK from:
   http://www.microsoft.com/ddk/ddk98.htm
   Install as D:\98DDK or another drive of your choice.

6. In Visual Studio, add the following to the list of directories, 
   EXACTLY as ordered (NTDDK must be before 98DDK) and at the TOP of the
   list before the default directories.  This is VERY IMPORTANT, or the 
   project won't compile properly.  You can substitute the drive letter "D:", 
   however, with the drive that the files are in on your particular system.

   Visual Studio->Tools->Options->Directories->Include Files
      D:\NTDDK\inc
      D:\NTDDK\inc\ddk
      D:\NTDDK\inc\win_me
      D:\98DDK\inc\win98

   Visual Studio->Tools->Options->Directories->Library Files
      D:\NTDDK\libfre\i386
      D:\NTDDK\lib\win_me
      D:\98DDK\lib\i386\free
      D:\98DDK\lib\win98

   Visual Studio->Tools->Options->Directories->Executable Files
      D:\98ddk\bin\win98

7. Do a "Build All"

8. You have a ready-to-run ready-to-distribute dTV package in the 
  ..\Release directory.  (Or the ..\Debug directory, if you
  selected the Debug Configuration)

9. If you make useful changes to the code, please make your changes
   to the LATEST source code and submit to dtv@marky.com
   

---END---
