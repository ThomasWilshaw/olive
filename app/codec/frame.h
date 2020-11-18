/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2020 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef FRAME_H
#define FRAME_H

#include <memory>
#include <QVector>

#include "common/rational.h"
#include "render/color.h"
#include "render/videoparams.h"

namespace olive {

class Frame;
using FramePtr = std::shared_ptr<Frame>;

/**
 * @brief Video frame data or audio sample data from a Decoder
 */
class Frame
{
public:
  Frame();

  static FramePtr Create();

  const VideoParams& video_params() const;
  void set_video_params(const VideoParams& params);

  static int generate_linesize_bytes(int width, VideoParams::Format format, int channel_count);

  int linesize_pixels() const
  {
    return linesize_pixels_;
  }

  int linesize_bytes() const
  {
    return linesize_;
  }

  int width() const
  {
    return params_.effective_width();
  }

  int height() const
  {
    return params_.effective_height();
  }

  VideoParams::Format format() const
  {
    return params_.format();
  }

  int channel_count() const
  {
    return params_.channel_count();
  }

  Color get_pixel(int x, int y) const;
  bool contains_pixel(int x, int y) const;
  void set_pixel(int x, int y, const Color& c);

  /**
   * @brief Get frame's timestamp.
   *
   * This timestamp is always a rational that will equate to the time in seconds.
   */
  const rational& timestamp() const
  {
    return timestamp_;
  }

  void set_timestamp(const rational& timestamp)
  {
    timestamp_ = timestamp;
  }

  /**
   * @brief Get the data buffer of this frame
   */
  char* data()
  {
    return data_.data();
  }

  /**
   * @brief Get the const data buffer of this frame
   */
  const char* const_data() const
  {
    return data_.constData();
  }

  /**
   * @brief Allocate memory buffer to store data based on parameters
   *
   * For video frames, the width(), height(), and format() must be set for this function to work.
   *
   * If a memory buffer has been previously allocated without destroying, this function will destroy it.
   */
  bool allocate();

  /**
   * @brief Return whether the frame is allocated or not
   */
  bool is_allocated() const
  {
    return !data_.isEmpty();
  }

  /**
   * @brief Destroy a memory buffer allocated with allocate()
   */
  void destroy()
  {
    data_.clear();
  }

  /**
   * @brief Returns the size of the array returned in data() in bytes
   *
   * Returns 0 if nothing is allocated.
   */
  int allocated_size() const
  {
    return data_.size();
  }

  FramePtr convert(VideoParams::Format format) const;

private:
  VideoParams params_;

  QByteArray data_;

  rational timestamp_;

  int linesize_;

  int linesize_pixels_;

};

}

Q_DECLARE_METATYPE(olive::FramePtr)

#endif // FRAME_H
