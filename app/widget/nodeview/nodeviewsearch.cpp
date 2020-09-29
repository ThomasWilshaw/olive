/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019 Olive Team

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

#include "nodeviewsearch.h"

#include "core.h"
#include "node/node.h"
#include "node/factory.h"
#include "nodeviewundo.h"

OLIVE_NAMESPACE_ENTER

NodeViewSearch::NodeViewSearch(QWidget* parent) :
	QLineEdit(parent),
    parent_(parent)
{
  this->hide();
  node_types_ = NodeFactory::NodeTypes();

  comp_ = new QCompleter(node_types_.keys(), this);
  comp_->setCaseSensitivity(Qt::CaseInsensitive);
  comp_->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  comp_->setMaxVisibleItems(3);
  this->setCompleter(comp_);

  connect(this, &NodeViewSearch::returnPressed, this, &NodeViewSearch::GenerateNode);
};

void NodeViewSearch::PopUp()
{
  QPoint p = parent_->mapFromGlobal(QCursor::pos());
  p.setX(p.x() - this->width());
  p.setY(p.y() - this->height());

  this->move(p);
  this->setText("");
  this->show();
  this->setFocus();
  this->activateWindow();
}

void NodeViewSearch::SetGraph(NodeGraph* graph)
{
  graph_ = graph;
}

void NodeViewSearch::SetScene(NodeViewScene* scene)
{
  scene_ = scene;
}

void NodeViewSearch::GenerateNode()
{
  QString value;
  if (this->text() != QString("")) {
    Node* new_node = NodeFactory::CreateFromID(node_types_[comp_->currentCompletion()]);
    if (new_node) {
      emit CreateNode(new_node);
    }
  }

  this->hide();
  this->setText("");
  parent_->setFocus();
}

OLIVE_NAMESPACE_EXIT
