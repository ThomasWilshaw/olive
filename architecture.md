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

- [OpenColorIO v2.0](https://opencolorio.org/) for color management
- [OpenImageIO](https://github.com/OpenImageIO/oiio) for still image input and output
- [OpenEXR](https://www.openexr.com/) for the internal disk cache
- [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)
  for timeline interchange

Notably, it is not based on the MLT framework unlike several other open source
video editors. It utilizes GPU acceleration throughout for high performance.

Olive also uses:

- [Qt 5](https://doc.qt.io/qt-5/) for the UI and various data structures,
  and more generally as platform abstraction
- [FFmpeg](https://ffmpeg.org/) for all video and audio encoding and decoding
- [CMake](https://cmake.org/) as its build system

## Key Goals

- Accurate pixel / color management
- Node-based compositing
- Fast workflow and rendering
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
of systems (see this [performance analysis](https://blender.stackexchange.com/a/148466)).
The cached frames are given a hash which means footage that is reused or moved
around does not need to be recached, allowing for a fairly seamless editing
experience once all media has been cached.

There is also a small playback cache that preloads a handful of frames ahead
of the cursor during playback. If pre-rendering cannot keep up with the
playback speed, then Olive falls back to real-time rendering. It also does
so while you tweak parameters to provide quick feedback.

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

## Code Layout

A shallow view of the directory structure of the code base which should give a
reasonable indications of where to look for code.

The code base is fairly inconsistently documented. Some header files have
extremely detailed comments and others have none at all, however most function
names are fairly explicit.

- `/app`

  All the source code is contained in this folder.

  - `core.cpp`

    The main central Olive application instance. Allows for a lot of the interaction between 
    various sections of the code base.

  - `main.cpp`

    Initializes Qt and various other libraries as well as starting the Command Line Interface
    if it is required.

  - `/audio`

    Contains code for managing audio playback and display.

  - `/cli`

    Manages the command line interface.

  - `/codec`
    The code here manages the encoding and decoding of all media (video, images and audio). Olive
    caches all footage as OpenEXR files for processing/storing on disk. Audio files are converted to 
    PPM files. The decoder class gives a simple interface to retrieve full frames of audio of video 
    data without having to deal with the complexities of the various codecs Olive can handle.

    We first probe a file to check it is valid and retrieves various useful metadata. If this is 
    successful, any streams in the footage can be opened and accessed (some formats contain for 
    instance audio and video streams). Image data is stored in a `FramePtr` and audio in a 
    `SampleBuffer`.

  - `/common`

    Contains various small useful classes that are used all over the code base.

  - `/config`

    The config handles all the program's system settings and is saved to an xml file.

  - `/dialog`

    Dialogs are any pop up window such as a color picker or the about text. Dialogs often run tasks
    such as exporting the final video or importing an OTIO file (see `/task`)

  - `/node`

    Code for the node system. This includes the nodes themselves, processing and the graph logic.

  - `/packaging`

    Platform specific files for compiling and instillation.

  - `/panel`

    UI layout code for Olive's various panels (viewer, timeline, node editor etc.).

  - `/project`

    Handles assets a settings for a specific project, these include:
    - Footage
    - Sequences
    - Folders
    - Project Settings
    - Window Layout

  - `/render`

    Rendering code including color management, threading, OpenGl processing and frame caching.

  - `/shaders`

    Fragment and vertex shaders. Mostly for nodes but includes includes anything that is OpenGL 
    accelerated such as the waveform and histogram.

  - `/task`

    Tasks are jobs that run in a separate thread such as pre caching or importing/exporting a project.
    Tasks can be either blocking or none blocking.

  - `/threading`

    Multi-threading management. Olive uses a ticket system, a ticket is a representative for a queued
    render job that other parts of the program can use when requesting video or audio. When you request
    a render job from the renderer, it returns a ticket. Most of the time, you'll want to receive a 
    signal when that ticket is done, however connecting a signal after you receive the ticket is a 
    potential race condition, since the ticket may finish before the signal is connected. A watcher is 
    a separate convenience class to solve this issue. It does some extra mutexing so that if you 
    connect it and then give it a ticket, you can be sure that the signal is emitted whether the 
    ticket is already finished or not.

  - `/timeline`

    Manages tracks and in/out points on the timeline.

  - `/tool`

    Enum of the various tools that can be used across various panels. This includes the pointer and 
    hand tools as well as area specific tools such as adding transitions in the timeline.

  - `/ts`

    Translation code. At time of writing this just includes en_US but there are various community 
    submitted translations in the pipeline. Contributions are welcome.

  - `/ui`

    Contains SVGs and other graphics used in Olive's UI as well as the Qt code they require.

  - `/undo`

    Undo stack code and a base class for all undo commands. Olive uses Qt's undo system and any 
    undoable command has to be created as a undo class the also redoes the command. See Qt's 
    documentation for more details.

  - `/widget`

    All of Olive's custom widgets. Widgets extend Qt's QWidget class and quite a few are based 
    off existing Qt widgets.

    It is worth noting that panels are just widgets the inherit from the panel base class.

  - `/window`

    Layout for the main window, toolbar and status bar.

- `/cmake`

  CMake files for discovering the required and optional libraries and some Mac OS app info.

  __Required__
  - FFmpeg
  - OpenColorIO
  - OpenEXR
  - OpenImageIO

  __Optional__
  - GoogleCrashpad
  - OpenTimelineIO

- `/docker`

  Contains the Docker files for Olive's Linux continuous integration, see the README for more 
  details. Based on the 
  [Academy Software Foundation's Docker](https://github.com/AcademySoftwareFoundation/aswf-docker) 
  implementation.