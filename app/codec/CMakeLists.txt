# Olive - Non-Linear Video Editor
# Copyright (C) 2021 Olive Team
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

add_subdirectory(ffmpeg)
add_subdirectory(oiio)

set(OLIVE_SOURCES
  ${OLIVE_SOURCES}
  codec/decoder.h
  codec/decoder.cpp
  codec/encoder.h
  codec/encoder.cpp
  codec/exportcodec.h
  codec/exportcodec.cpp
  codec/exportformat.h
  codec/exportformat.cpp
  codec/frame.h
  codec/frame.cpp
  codec/samplebuffer.h
  codec/samplebuffer.cpp
  codec/waveinput.h
  codec/waveinput.cpp
  codec/waveoutput.h
  codec/waveoutput.cpp
  PARENT_SCOPE
)
