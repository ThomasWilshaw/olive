# Olive Architecture

This document outlines Olive from a technical point of view. It is meant as a
starting point for new developers to understand its core design and code
organization.

If you are looking for a user guide, see
[Olive 0.2.x Quickstart](https://github.com/olive-editor/olive/wiki/Olive-0.2.x-Quickstart)
instead.

## High Level View

Olive is a fully color managed, node-based non-linear video editor (NLE) in the
making. It is currently in alpha stage and thus undergoing rapid change.

Olive is open source and cross-platform (Linux, Windows, macOS).
It has been written in modern C++ from the ground up using cutting edge
technologies. These include:

- [OpenColorIO v2.0](https://opencolorio.org/) (OCIO) for color management
- [OpenImageIO](https://github.com/OpenImageIO/oiio)
  (OIIO) for still image input and output
- [OpenEXR](https://www.openexr.com/) for the internal disk cache
- [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)
  (OTIO) for timeline interchange

Notably, it is not based on the MLT framework unlike several other open source
video editors. It utilizes GPU acceleration throughout for high performance.

Olive also uses:

- [Qt 5](https://doc.qt.io/qt-5/) for the user interface (UI), various data
  structures, and more generally as platform abstraction
- [OpenGL](https://www.khronos.org/registry/OpenGL/index_gl.php) as graphics
  API and its shading language GLSL
- [FFmpeg](https://ffmpeg.org/) for all video and audio encoding and decoding
- [CMake](https://cmake.org/) as its build system

## Key Goals

- Accurate pixel / color management
- Node-based compositing
- Efficient workflow and rendering
- Easy interchange
- Be able to run on relatively low spec hardware

## Core Components

The code base can be more or less split into the followings groups:

1. Ingestion and caching
2. Node system
3. UI
4. Color Management

### Ingestion and Caching

Olive uses FFmpeg and OpenImageIO (OIIO) to read a large variety of input media.
The decoder class removes all the complexities of opening and reading various
codecs. The decoder always returns a complete frame that may need to be
converted to the correct pixel format. See `decoder.h` for more details.

A key feature of Olive is the on disk caching system which allows for real-time
playback with even the most complex of node setups. All frames are cached as
linear EXRs which can be read from disk in real-time in even the most minimal
of systems (see this [performance analysis](https://blender.stackexchange.com/a/148466)). By default
Olive has a single cache folder that is used across all projects, this means that a footage cached
anywhere is available in any project, this also means you can pre cache all you footage before using
it anywhere.

The cached frames are given a unique hash, hashing is a way to determine whether a frame has 
changed (or more specifically, to generate a unique ID for every frame) without doing any actual
image generation, which is very expensive. The hash is also dependant on the cache resolution, i.e.
if you change from a half resolution cache to a quarter resolution the footage will need to be re
cached. We employ a lot of optimizations to prevent unnecessarily changing the hash and forcing a 
re-render.

For example, "track" nodes don't affect the resulting image at all so they don't add to the hash.
All they do is forward to the clip at that time. Likewise, the clip itself doesn't affect the image,
it just transforms the time, so it doesn't add to the hash either. Effectively that means if you
have footage -> clip -> track -> viewer, the footage is the only node that actually gets hashed (at
a time transformed by the track and clip), meaning that footage can be anywhere in the sequence, and
as far as the hash is concerned (as long as no other nodes have affected the final image), the hash
is the same. That's how pre-caching works, it caches a footage node at the sequence's resolution and
the rest of the program is able to re-use it.There are more optimizations than that, but those are
the biggest ones.

There is also a small playback cache that preloads a handful of frames ahead of the cursor during
playback. If pre-rendering cannot keep up with the playback speed, then Olive falls back to
real-time rendering. It also does so while you tweak parameters to provide quick feedback.

### Node System

The core of Olive, and what makes it unique amongst video editors, is the node
system. The entire project data is stored in a single graph internally. Some of
it is exposed as node graph in the interface, primarily to define and visualize
the data flow. Nodes are like visual programming and allow a user to build an
arbitrarily complex setup.

From an architecture point of view the node system consists of the following
parts:

- The graph which manages adding, removing and connecting nodes together
- The nodes themselves
- The node table

The graph is a Direct Acyclic Graph (DAG) which means each edge allows data to
flow in only one direction and that feedback loops / cycles are not allowed.
A node does not have to be connected to be part of the graph. In fact, when
nodes are added with `NodeGraph::AddNode()` they are not connected by default.
It is also important to note that when a node is removed with `NodeGraph::TakeNode()`
it is unconnected and removed from the graph but not destroyed. The class
`NodeGraph` also contains a few helper functions.

The `Node` class is a simple base class for all nodes that is intended to be
sub-classed. It gives access to various pieces of metadata about the node such
as its ID, name and description as well as providing a large number of helper
functions. The class' header file is very well documented and worth reading. 

<!--
TODO:
- Explain different node types and how they inter connect
- Explain node tables
-->

### UI

Olive relies heavily on Qt for the UI code (as well as various data types).
The UI is split into movable, dock-able panels (leveraging `QDockWidget`) that
allow for a very customizable interface.

### Color Management

If you're not familiar with color management, some good places to start are
[Cinematic Color](https://cinematiccolor.org/) and
[The Hitchhiker's Guide to Digital Colour](https://hg2dc.com/).

Unlike most video editing software, Olive is not based on outdated, hard coded
color assumptions but instead has a scene-linear color pipeline. This means
that all color operations can happen in a physically correct way and that Olive
can handle wide gamuts and high dynamic range imagery.

Whilst color management is woven into the entire code base the two main classes that deal with it
are `colormanager.h` and `colorprocessor.h`. To understand these classes you need to first
understand how OCIO works in principle. OCIO uses config files (see the folder `ocioconf` for 
Olive's built in default config, user configs can be used instead) that define all the color spaces it 
knows about as transforms to and from a reference space (usually a linear, wide gamut space such as
Rec.2020 or ACES CG).

When footage is imported into Olive it is run through the corresponding transforms to get it into
this reference space. Most image manipulation happens in this reference space. When pixels need to
be displayed they are run through an output transform which takes them from the reference space to a
display space such as sRGB. Bear in mind this is a very simplified explanation and see the
[OCIO documentation](https://opencolorio.readthedocs.io/en/latest/) for the full details.

To access the color spaces available in the config (whether for building transforms or for menus in
the UI) use `Colormanger` class and to actually use a create a transform and use it, use the
`ColorProcessor` class.

The main takeaway here is that anytime pixels are modified, displayed or otherwise accessed it is 
very important to be aware of what color space they are in and what space the need to be in to do
the required operation. 

## Code Layout

A shallow view of the directory structure of the code base which should give a
reasonable indications of where to look for code.

The code base is fairly inconsistently documented. Some header files have
extremely detailed comments and others have none at all, however most function
names are fairly explicit.

- `/.github`

  Files related to GitHub.com features like GitHub Actions workflows and
  issue templates.

- `/app`

  All the application source code is contained in this folder.

  - `main.cpp`

    Initializes Qt and various other libraries. It also contains the command
    line parsing to decide whether Olive should run with a graphical interface
    or in headless mode (incomplete).

  - `core.cpp`

    The central Olive application instance, started by `main.cpp`. Allows for a
    lot of the interaction between various sections of the code base.

  - `/audio`

    Contains code for managing audio playback and display.

  - `/cli`

    Functionality related to the command line interface (incomplete).

  - `/codec`

    The code here manages the encoding and decoding of all media (video, images
    and audio). Olive caches all footage as OpenEXR files on disk. Audio files
    are converted to PPM files. The decoder class gives a simple interface to
    retrieve full frames of audio or video data without having to deal with the
    complexities of the various codecs Olive can handle.

    We first probe a file to check if it is valid and retrieve various useful
    metadata. If this is successful, any streams in the medium can be opened
    and accessed. Some file formats contain multiple audio, video, subtitle and
    other streams. Image data is stored in a `FramePtr` and audio in a 
    `SampleBuffer`.

  - `/common`

    Contains various utility classes that are used all over the code base, such
    as file functions, math helpers and custom data types.

  - `/config`

    This code handles all the program's system settings and is saved to an XML
    file.

  - `/dialog`

    Dialogs are any pop-up window such as a color picker or the about box.
    Dialogs often run tasks such as exporting the final video or importing an
    OTIO file (see `/task`).

  - `/node`

    Code for the node system. This includes the nodes themselves, processing
    and the graph logic.

  - `/packaging`

    Platform-specific files for creating packages:
    - AppImage for Linux
    - NSIS installer and a portable version for Windows
    - Application bundle (`.app`) for macOS

    Note that the metadata template file for macOS is at
    `/cmake/MacOSXBundleInfo.plist.in`.

  - `/panel`

    UI layout code for Olive's various panels (viewer, timeline, node editor etc.).

  - `/project`

    Handles assets and settings for specific projects. These include:
    - Footage
    - Sequences
    - Folders
    - Project Settings
    - Window Layout

    Also contains the code for saving and loading project files.

  - `/render`

    Code color management, threading, frame caching and the OpenGL renderer.

  - `/shaders`

    Fragment and vertex shaders written in OpenGL Shading Language (GLSL).
    Mostly for the user-visible nodes that enable compositing and effects, but
    includes anything that is GPU accelerated such as the waveform monitor and
    histogram.

  - `/task`

    Tasks are jobs that run in a separate thread such as pre-caching or
    importing/exporting a project. Tasks can be either blocking or non-blocking.

  - `/threading`

    Multi-threading management. Olive uses a ticket system. A ticket represents
    a queued render job that other parts of the program can use when requesting
    video or audio. When you request a render job from the renderer, it returns
    a ticket. Most of the time, you will want to receive a signal when that
    ticket is done, however connecting a signal after you receive the ticket is
    a potential race condition, since the ticket may finish before the signal
    is connected. A watcher is a separate convenience class to solve this issue.
    It does some extra mutexing so that if you connect it and then give it a
    ticket, you can be sure that the signal is emitted whether the ticket is
    already finished or not.

  - `/timeline`

    Manages tracks and in/out points on the timeline.

  - `/tool`

    Enum of the various tools that can be used across various panels. This
    includes the pointer and hand tool as well as area-specific tools such as
    adding transitions in the timeline.

  - `/ts`

    Translation source files. The language used in Olive's code base is
    American English (`en_US`), but it uses
    [Qt's internationalization](https://doc.qt.io/qt-5/internationalization.html)
    capabilities to support multiple languages in the UI. There are several
    community submitted translations in the pipeline. Contributions are welcome.
    See [Translating Olive](https://github.com/olive-editor/olive/wiki/Translations)
    for details on the process.

  - `/ui`

    Contains SVG files and other graphics used in Olive's UI as well as the Qt
    code they require.

  - `/undo`

    Undo stack code and a base class for all undo commands. Olive uses
    [Qt's undo framework](https://doc.qt.io/qt-5/qundo.html). Any undoable
    command has to be created as an undo class that also redoes the command.

  - `/widget`

    All of Olive's custom widgets. Widgets extend
    [Qt's QWidget](https://doc.qt.io/qt-5/qtwidgets-index.html) class and quite
    a few are based off existing Qt widgets.

    It is worth noting that panels are just widgets which inherit from the
    panel base class.

  - `/window`

    Layout for the main window, toolbar and status bar.

- `/cmake`

  CMake modules for discovering required and optional [dependencies](#dependencies).
  Note that CMake also has built-in modules for common libraries and tools.

- `/docker`

  Contains the Docker files for Olive's Linux build container, which is used for
  continuous integration (CI) / nightly builds.
  Also see the [Docker README](docker/README.md).

## Dependencies

- __Qt__ >= 5.6
  - Qt Base (Core, GUI, Widgets etc.)
  - Qt Multimedia (audio input/output)
  - Qt Tools (translation, packaging)
  - Qt SVG (currently unused)
- __OpenColorIO__ 2.0.0
- __OpenImageIO__ 2.1.12
- __OpenEXR__
- __FFmpeg__ >= 3.0
  - avutil
  - avcodec
  - avformat
  - avfilter
  - swscale
  - swresample
- __OpenTimelineIO__ (optional)
- __Google Crashpad__ (optional)
- __Breakpad__ (optional, only `minidump_stackwalk` binary)
- __OpenGL__ (`GL/gl.h` on Linux)

## Build Environment

- __Git__
- __CMake__ >= 3.13
- __x64 Toolchain__ (GCC, Clang, MSVC)
- __Doxygen__ (optional)
