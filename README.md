# HelloCube
The purpose of this little program is to demonstrate the basic 
usage of modern OpenGL core profiles. The program will display
a rotating, unlit cube, with some color transition on each face.

![Screenshot of HelloCube application: a cube](https://raw.githubusercontent.com/derhass/HelloCube/master/screenshot.png)


### Used Libraries

Besides OpenGL itself, the following libraries are used:
* glm (http://glm.g-truc.net/), the OpenGL Mathematics library is used for matrix and vector math. It is implemented as a header-only C++ library and also completely incorporated into this project, so that no external glm library is required.
* GLFW3 (http://www.glfw.org/) is used as a multi-platform library for creating windows and OpenGL contexts and for event handling. For 32Bit Windows, the static libraries built with Visual Studio 2010 to 2013 as well as
mingw are included (together with the header files), so that no extra libraries have to be installed here. For other platforms, you should install the GLFW library >= 3.0 manually.

Furthermore, glad (https://github.com/Dav1dde/glad) was used to generate the C code for the OpenGL
loader. This loader acquires all of the pointers to the OpenGL function at runtime. The loader source
code is directly integrated into this project, glad itself is neither required as a build nor runtime
dependency. However, you might use it to re-generate the loader if a new OpenGL version or extension
emerges (if you intend to use the new functionality).

### Requirements

For windows, project files for Visual Studio 2010 to 2015 are provided. They generate statically linked
binaries which should run out-of-the-box even when copied to another system. For Linux with GNU make,
a simple Makefile is provided. It tries to findthe GLFW3 library via pkg-config.

To run this program, you need an OpenGL implementation supporting (at least) GL 3.2 
in core profile. This means you'll need one of those GPUs:
* Intel HD Graphics 2000 (introduced with Sandy Bride in 2011) or better,
* NVidia Geforce 8 (introduced 2006) or better, or
* AMD/ATI Radeon HD 2000 (introduced 2006) or better.

Make sure your GPU drivers are up to date.

### Shaders

A couple of demo shaders is provided in the `shaders` subdirectory. They can be switched
at runtime using the number keys 0 to 9. Note that the shaders are reloaded, recompiled and
relinked at the key press, so you can edit the shaders while the main programm is running.

Have fun!


