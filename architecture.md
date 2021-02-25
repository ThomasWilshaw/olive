# Olive code Architecture

## High level view

Olive is a Qt based program. The code base can be more or less split into 4 sections:

1. Ingestion and caching,
2. Node system,
3. UI,
4. Colour Management

### 1. Ingestion and Caching

Olive uses FFMpeg and OpenImageIO (OIIO) to read a large variety of inout media. The decoder class
removes all the complexities of opening and reading various codecs. The decoder always returns a
complete frame that may need to be converted to the correct pixel format. See `decoder.h` for more
details.

A key feature of Olive is the on disk caching system which allows for relatime playback with even
the most complex of node setups ([see here](https://blender.stackexchange.com/a/148466) for a 
detailed overview of how quiockle EXRs can read and write). All frames are cached as linear EXRs 
whoch can be read from disk in realtime in even the most minimialof systems. The cached frames are 
given a hash which means footage that is reused or moved around does not need ot be recached, 
allowing for a fairly seemless editing experience once all media has been cached.

There is also a small playback cache that preloads a handfull of frames ahead of the cursor during
playback. 

### 2. Node System

The core of Olive, and what makes it unique amongst video editors, is the node system; the entire
project is stored in a single graph. Nodes are like visual programming and allow a user to build
an arbitarily complex set up. From an architectre point of view the node system consists of the
following parts:
 - The graph whoch manages adding, removing and connecting nodes together,
 - The nodes themselves,
 - The node table

The graph is a Direct Acyclic Graph (DAG) which means each edge allows data to flow in only one
direction and that feedback loops are not allowed. A node does not have to be connected to be part
of the graph and infact when nodes are added with `NodeGraph::AddNode()` they are not connected by
default. It is also inportant to note that when a node is removed with `NodeGraph::TakeNode()` it 
is uncconected and removed from the graph but not destroyed. The class `NodeGraph` also contains a 
few helper functions.

The `Node` class is a simple base class for all nodes that is intended to be subclassed. It gives
acces to various pieces of metedata about the node such as it's ID, name and description as well 
as providing a alarge number of helper functions. The classes header file is very well documented
and worth reading. 

TODO: 
- Explain different node types and how they inter connect
- Explain node tables



### 3. UI

Olive relies heavily on Qt for the UI code (as well as various data types). The UI is split into
movable, docakble panels (leveragnig QDockWidget) that allow for a very customisable interface. 

### 4. Colour Management
TODO

## Code Map
An fairly shallow view of the directory structure of the code base which should give a reasonable
indications of where to look for code. The code base is fairl inconsistently documented, some
header files have extreamly detailed comments and others have none at all, however most function
names are fairly explicit.
## /app
All the source code is contained in this folder.

### /audio
Contains code for managing audio playback and display.

### /cli
Manmges the command line interface.

### /codec
The code here manages the encoding and decoding of all media (video, images and audio). Olive
caches all footage as OpenEXR files for processing/storing on disk. Audio files are converted to 
PPM files. The decoder class gives a simple interface to retrieve full frames of audio of video 
data without having to deal with the complexities of the various codecs Olive can handle.

We first probe a file to check it is valid and retireivs various useful metadata. If this is 
succesful, any streams in the footage can be opened and accessed (some formats contain for 
instance audio and video streams). Image data is stored in a `FramePtr` and audio in a 
`SampleBuffer`.

### /common
Contains various small useful classes that are used all over the code base.

### /config
The config handles all the program's system settings and is saved to an xml file.

### /dialog
Dialogs are any pop up window such as a colour picker or the about text. Dialogs often run tasks
such as exporting the final video or inporting an OTIO file (see [/task](#/task)).

### /node
Code for the node system. This includes the nodes themselves, processing and the graph logic.

### /packaging
Platform specific files for compiling and instillation.

### /panel
UI layout code for Olive's various panels (viewer, timeline, node editor etc.).

### /project
Handles assets a settings for a specific project, these include:
 - Footage
 - Sequences
 - Folders
 - Project Settings
 - Window Layout

### /render
Rendering code including colour management, threading, OpenGl processing and frame caching.

### /shaders
Fragment and vertex shaders. Mostly for nodes but includes includes anything that is OpenGL 
accelerated such as the waveform and histogram.

### /task
Tasks are jobs that run in a seperate thread such as pre caching or importing/exporting a project.
Tasks can be either blocking or none blocking.

### /threading
Multithreading managment. Olive uses a ticket system, a ticket is a representative for a queued
render job that other parts of the program can use when requesting video or audio. When you request
a render job from the renderer, it returns a ticket. Most of the time, you'll want to receive a 
signal when that ticket is done, however connecting a signal after you receive the ticket is a 
potential race condition, since the ticket may finish before the signal is connected. A watcher is 
a separate convenience class to solve this issue. It does some extra mutexing so that if you 
connect it and then give it a ticket, you can be sure that the signal is emitted whether the 
ticket is already finished or not.

### /timeline
Manages tracks and in/out points on the timeline.

### /tool
Enum of the various tools that can be used across various panels. This includes the pointer and 
hand tools as well as area specific tools such as adding transitions in the timeline.

### /ts
Translation code. At time of writing this just includes en_US but there are various community 
submitted translatiopns in the pipeline. Contributions are welcome.

### /ui
Contains SVGs and other graphics used in Olive's UI as well as the Qt code they require.

### /undo
Undo stack code and a base class for all undo commands. Olive uses Qt's undo system and any 
undoable command has to be created as a undo class the also redoes the command. See Qt's 
dcumentation for more details.

### /widget
All of Olive's custom widgets. Widgets extend Qt's QWidget class and quite a few are based 
off existing Qt widgets.

It's worth noting that panels are just widgets the inherit from the panel base class.

### /window
Layout for the main window, toolbar and status bar.

### core.cpp
The main central Olive application instance. Allows for a lot of the interaction between 
various sections of the code base.

### main.cpp
Initialises Qt and various other libraries as well as starting the Command Line Interface
if it is required.

## /cmake
CMake files for discovering the required and optional libraries.

 __Required__
 - FFmpeg
 - OpenColorIO
 - OpenEXR
 - OpenImageIO

 __Optional__
  - GoogleCrashpad
  - OpenTimelineIO

  _What is the MACOs stuff for?_

## /docker
Contains the Docker files for Olive's Linux continous intergration, see the README for more 
details. Based on the 
[Academy Software Foundation's Docker](https://github.com/AcademySoftwareFoundation/aswf-docker) 
implementation .

## Files

The other files are fairly self explanatory.