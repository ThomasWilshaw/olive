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

#ifndef SHADERVALUE_H
#define SHADERVALUE_H

#include "node/param.h"

namespace olive {

struct ShaderValue
{
  ShaderValue()
  {
    type = NodeParam::kNone;
    array = false;
  }

  ShaderValue(QVariant data_in, NodeParam::DataType type_in, bool array_in = false)
  {
    data = data_in;
    type = type_in;
    array = array_in;
  }

  NodeParam::DataType type;
  QVariant data;
  bool array;

  QString tag;

};

using NodeValueMap = QHash<QString, ShaderValue>;

}

#endif // SHADERVALUE_H
