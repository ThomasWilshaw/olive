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

#ifndef NODEVIEWSEARCH_H
#define NODEVIEWSEARCH_H

#include <QCompleter>
#include <QLineEdit>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "common/define.h"
#include "node/graph.h"
#include "nodeviewscene.h"

OLIVE_NAMESPACE_ENTER

class NodeViewSearch : public QLineEdit
{
  Q_OBJECT
public:
  NodeViewSearch(QWidget* parent = nullptr);

  void PopUp();

  void SetGraph(NodeGraph* graph);

  void SetScene(NodeViewScene* scene);

  void Disappear();

signals:
  void CreateNode(Node* node);

protected:
  virtual void focusOutEvent(QFocusEvent*) override;

  virtual void leaveEvent(QEvent* event) override;

  virtual bool eventFilter(QObject* object, QEvent* event) override;

  //Required to let event filter work
  virtual void mouseMoveEvent(QMouseEvent* e) override { e->ignore(); }

  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  void GenerateNode();

  QCompleter* comp_;

  QMap<QString, QString> node_types_;

  NodeGraph* graph_;

  NodeViewScene* scene_;

  QWidget* parent_;

};

OLIVE_NAMESPACE_EXIT

#endif //NODEVIEWSEARCH
