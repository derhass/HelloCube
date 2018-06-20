This is GLFW-3.2 from http://www.glfw.org/

The provided windows binaries are NOT from the official 32 Bit Windows build
( https://github.com/glfw/glfw/releases/download/3.2/glfw-3.2.bin.WIN32.zip ),
but compiled locally with following differences to the official ones:
- the CRT libraries were changed from the DLL version to static,
  resulting in the final application will not depend on the MSVCRT*.DLLs
  (the application itself must of course also be compiled with the static CRT)
- for each VC version, two libraries are provided, one linked against
  the debug CRT (suffix d), and one linked against the relase CRT.
- the debug version is a debug build, but with the PDB information removed
- the directory structure has been changed to lib-vc\$(VisualStudioVersion)
  so that we can use the $(VisualStudioVersion) variable in the .vcxproj
  file and use the same project file for different Visual Studio versions.
- the DLL versions are not provided here.
The resulting .EXE should be self-contained and run out-of-the-box when 
moved to another Windows system than the one it was built on...

The provided Linux binary was compiled on a Debian 7.11 (Wheezy) as
x86_64 with gcc 4.7.2, and linked against Wheezy's default X11 libraries.
It is recommended to use the native GLFW3 package of your Linux distribution
instead. By default, the project Makefile will use the pkg-config tool
to find the local installation of the GLFW3 library on your system. Only if
your run make with USE_PKGCONFIG=0, the binary provided here will be used.

