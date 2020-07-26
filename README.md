# HelloCube
The purpose of this little program is to demonstrate the basic 
usage of modern OpenGL core profiles. The program will display
a rotating, unlit cube, with some color transition on each face.

![Screenshot of HelloCube application: a cube](https://raw.githubusercontent.com/derhass/HelloCube/master/screenshot.png)


### Used Libraries

Besides OpenGL itself, the following libraries are used:
* glm (http://glm.g-truc.net/), the OpenGL Mathematics library is used for matrix and vector math. It is implemented as a header-only C++ library and also completely incorporated into this project, so that no external glm library is required.
* GLFW3 (http://www.glfw.org/) is used as a multi-platform library for creating windows and OpenGL contexts and for event handling. For 32Bit and 64Bit Windows, the static libraries built with Visual Studio 2010 to 2017 as well as
mingw are included (together with the header files), so that no extra libraries have to be installed here. For other platforms, you should install the GLFW library >= 3.0 manually.

Furthermore, glad (https://github.com/Dav1dde/glad) was used to generate the C code for the OpenGL
loader. This loader acquires all of the pointers to the OpenGL function at runtime. The loader source
code is directly integrated into this project, glad itself is neither required as a build nor runtime
dependency. However, you might use it to re-generate the loader if a new OpenGL version or extension
emerges (if you intend to use the new functionality). Note that the included code was generated with the
(not yet released as stable) version 2 of glad (https://github.com/Dav1dde/glad/tree/glad2).

### Requirements

For windows, a Visual Studio project file is provided. It should work beginning with VS 2010 and
generates statically linked binaries which should run out-of-the-box even when copied to another system. For Linux with GNU make,
a simple Makefile is provided. It tries to find the GLFW3 library via pkg-config.

To run this program, you need an OpenGL implementation supporting (at least) GL 3.2 
in core profile. This means you'll need one of those GPUs:
* Intel HD Graphics 2000 (introduced with Sandy Bridge in 2011) or better,
* NVidia Geforce 8 (introduced 2006) or better, or
* AMD/ATI Radeon HD 2000 (introduced 2006) or better.

Make sure your GPU drivers are up to date.

### Shaders

A couple of demo shaders is provided in the `shaders` subdirectory. They can be switched
at runtime using the number keys 0 to 9. Note that the shaders are reloaded, recompiled and
relinked at the key press, so you can edit the shaders while the main programm is running.

### Command-Line arguments

The HelloCube program supports a number of command-line arguments.

#### Window control

* `--fullscreen`: run in fullscreen mode
* `--undecorated`: run without window decoration (can be used in combination with initial window size and position arguments to achieve "borderless windowed fullscreen")
* `--width $w`: set initial window width to `$w`
* `--height $h`: set initial window height to `$h`
* `--x $x`: set initial window position x coordinate to `$x`
* `--y $y`: set initial window position y coordinate to `$y`

#### [OpenGL debug output](https://www.khronos.org/opengl/wiki/Debug_Output)

This can be controlled by:
* `--gl-debug-level $level`
  * `0`: no debug output (the default)
  * `1`: debug output enabled, but report errors only
  * `2`: debug output enabled, report everything

Debug output will only work if we got a GL context with Versison 4.3 or higher, or if at least one of the
[`GL_KHR_debug`](https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt) or
[`GL_ARB_debug_output`](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_debug_output.txt)
extensions are supported.

#### Miscellaneous features

* `--frameCount $n`: exit application after `$n` frames were rendered

#### OpenGL Quadbuffer Stereo

A special version with support for Quadbuffer Stereo is provided separately
in the
[`quadbuffer_stereo` branch](https://github.com/derhass/HelloCube/tree/quadbuffer_stereo).

Have fun!
