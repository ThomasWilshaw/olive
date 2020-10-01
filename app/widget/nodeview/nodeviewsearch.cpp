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
  //Set Focus proxy to keep mouse evnt workin when completer pops up
  this->setCompleter(comp_);

  connect(this, &NodeViewSearch::returnPressed, this, &NodeViewSearch::GenerateNode);

  this->setMouseTracking(true);
  parent_->installEventFilter(this);
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

  Disappear();
}

void NodeViewSearch::Disappear() {
  this->hide();
  this->setText("");
  parent_->setFocus();
}

void NodeViewSearch::focusOutEvent(QFocusEvent*)
{
  Disappear();
}

void NodeViewSearch::leaveEvent(QEvent* event)
{ 
  //Disappear();
  // check not on popup menu eaither
}

bool NodeViewSearch::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::MouseMove) {
    if (this->isVisible()) {
      QPoint cursor_pos = QCursor::pos();
      QPoint this_pos = parent_->mapToGlobal(this->pos());
      
      if (cursor_pos.x() > (this_pos.x() + this->width()/2 + 100)) {
        Disappear();
      }
      if (cursor_pos.x() < (this_pos.x() + this->width() / 2 - 100)) {
        Disappear();
      }
      if (cursor_pos.y() < (this_pos.y() + this->height() / 2 - 50)) {
        Disappear();
      }
      if (cursor_pos.y() > (this_pos.y() + this->height() / 2 + 50)) {
        Disappear();
      }
    }
    return false;
  } else {
    // standard event processing
    return QObject::eventFilter(object, event);
  }
}

void NodeViewSearch::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape) {
    Disappear();
  }

  // https://doc-snapshots.qt.io/qt5-5.15/qcompleter.html#setCurrentRow plus arrow key to modify maybe?

  QLineEdit::keyPressEvent(event);
}

OLIVE_NAMESPACE_EXIT
