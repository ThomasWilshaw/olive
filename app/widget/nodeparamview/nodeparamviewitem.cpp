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

#include "nodeparamviewitem.h"

#include <QCheckBox>
#include <QDebug>
#include <QEvent>
#include <QPainter>

#include "core.h"
#include "config/config.h"
#include "nodeparamviewundo.h"
#include "nodeitemdock.h"
#include "project/item/sequence/sequence.h"


OLIVE_NAMESPACE_ENTER

NodeParamViewItem::NodeParamViewItem(Node *node, QWidget *parent) :
  QWidget(parent),
  node_(node),
  active_(true)
{
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setSpacing(0);
  main_layout->setMargin(0);

  // Create and add contents widget
  QVector<NodeInput*> inputs;

  // Filter out inputs
  foreach (NodeParam* p, node->parameters()) {
    if (p->type() == NodeParam::kInput) {
      inputs.append(static_cast<NodeInput*>(p));
    }
  }

  body_ = new NodeParamViewItemBody(inputs);
  connect(body_, &NodeParamViewItemBody::InputDoubleClicked, this, &NodeParamViewItem::InputDoubleClicked);
  connect(body_, &NodeParamViewItemBody::RequestSelectNode, this, &NodeParamViewItem::RequestSelectNode);
  connect(body_, &NodeParamViewItemBody::RequestSetTime, this, &NodeParamViewItem::RequestSetTime);
  connect(body_, &NodeParamViewItemBody::KeyframeAdded, this, &NodeParamViewItem::KeyframeAdded);
  connect(body_, &NodeParamViewItemBody::KeyframeRemoved, this, &NodeParamViewItem::KeyframeRemoved);

  main_layout->addWidget(body_);

  connect(node_, &Node::LabelChanged, this, &NodeParamViewItem::Retranslate);

  Retranslate();
}

void NodeParamViewItem::SetTimeTarget(Node *target)
{
  body_->SetTimeTarget(target);
}

void NodeParamViewItem::SetTime(const rational &time)
{
  time_ = time;

  body_->SetTime(time_);
}

Node *NodeParamViewItem::GetNode() const
{
  return node_;
}

void NodeParamViewItem::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);

  QPainter p(this);
  Node* node = GetNode();
  Color node_color = Config::Current()[QStringLiteral("NodeCatColor%1")
      .arg(node->Category().first())].value<Color>();

  // Draw border
  if (GetActive()) {
    p.setPen(node_color.toQColor());
    p.drawRect(0, 0, width() - 1, height() - 1);
  }
}

NodeParamViewItemBody* NodeParamViewItem::GetBody()
{
  return body_;
}

void NodeParamViewItem::SetActive(bool active)
{
  active_ = active;
}

bool NodeParamViewItem::GetActive()
{
  return active_;
}

void NodeParamViewItem::SignalAllKeyframes()
{
  body_->SignalAllKeyframes();
}

void NodeParamViewItem::changeEvent(QEvent *e)
{
  if (e->type() == QEvent::LanguageChange) {
    Retranslate();
  }

  QWidget::changeEvent(e);
}

void NodeParamViewItem::Retranslate()
{
  node_->Retranslate();
  /*
  if (node_->GetLabel().isEmpty()) {
    title_bar_lbl_->setText(node_->Name());
  } else {
    title_bar_lbl_->setText(tr("%1 (%2)").arg(node_->GetLabel(), node_->Name()));
  }
  */
  body_->Retranslate();
}

NodeParamViewItemTitleBar::NodeParamViewItemTitleBar(QWidget *parent) :
  QWidget(parent)
{
  // Probably better to do in the style sheet
  this->setStyleSheet("background-color: #404040");
}

void NodeParamViewItemTitleBar::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);

  QPainter p(this);
  Node* node = static_cast<NodeItemDockTitle*>(parent())->GetNode();
  Color node_color = Config::Current()[QStringLiteral("NodeCatColor%1")
      .arg(node->Category().first())].value<Color>();

  // Draw bottom border using text color
  int bottom = height() - 1;
  p.setPen(node_color.toQColor());
  //p.drawLine(0, bottom, width(), bottom);
  p.drawLine(0, 0, width(), 0);
  p.drawLine(0, 1, width(), 1);
}

NodeParamViewItemBody::NodeParamViewItemBody(const QVector<NodeInput *> &inputs, QWidget *parent) :
  QWidget(parent)
{
  int row_count = 0;

  const int max_col = 10;

  QGridLayout* content_layout = new QGridLayout(this);

  foreach (NodeInput* input, inputs) {
    InputUI ui_objects;

    // Add descriptor label
    ui_objects.main_label = new ClickableLabel();
    connect(ui_objects.main_label, &ClickableLabel::MouseDoubleClicked, this, &NodeParamViewItemBody::LabelDoubleClicked);

    if (input->IsArray()) {
      QHBoxLayout* array_label_layout = new QHBoxLayout();
      array_label_layout->setMargin(0);

      CollapseButton* array_collapse_btn = new CollapseButton();

      array_label_layout->addWidget(array_collapse_btn);
      array_label_layout->addWidget(ui_objects.main_label);

      content_layout->addLayout(array_label_layout, row_count, 0);

      NodeParamViewItemBody* sub_body = new NodeParamViewItemBody(static_cast<NodeInputArray*>(input)->sub_params());
      sub_bodies_.append(sub_body);
      sub_body->layout()->setMargin(0);
      content_layout->addWidget(sub_body, row_count + 1, 0, 1, max_col + 1);

      connect(array_collapse_btn, &CollapseButton::toggled, sub_body, &NodeParamViewItemBody::setVisible);

      connect(sub_body, &NodeParamViewItemBody::KeyframeAdded, this, &NodeParamViewItemBody::KeyframeAdded);
      connect(sub_body, &NodeParamViewItemBody::KeyframeRemoved, this, &NodeParamViewItemBody::KeyframeRemoved);
      connect(sub_body, &NodeParamViewItemBody::RequestSetTime, this, &NodeParamViewItemBody::RequestSetTime);
      connect(sub_body, &NodeParamViewItemBody::InputDoubleClicked, this, &NodeParamViewItemBody::InputDoubleClicked);
      connect(sub_body, &NodeParamViewItemBody::RequestSelectNode, this, &NodeParamViewItemBody::RequestSelectNode);
    } else {
      content_layout->addWidget(ui_objects.main_label, row_count, 0);
    }

    // Create a widget/input bridge for this input
    ui_objects.widget_bridge = new NodeParamViewWidgetBridge(input, this);

    // Add widgets for this parameter to the layout
    {
      int column = 1;
      foreach (QWidget* w, ui_objects.widget_bridge->widgets()) {
        content_layout->addWidget(w, row_count, column);
        column++;
      }
    }

    if (input->is_connectable()) {
      // Create clickable label used when an input is connected
      ui_objects.connected_label = new NodeParamViewConnectedLabel(input);
      connect(ui_objects.connected_label, &NodeParamViewConnectedLabel::ConnectionClicked, this, &NodeParamViewItemBody::ConnectionClicked);
      content_layout->addWidget(ui_objects.connected_label, row_count, 1);

      connect(input, &NodeInput::EdgeAdded, this, &NodeParamViewItemBody::EdgeChanged);
      connect(input, &NodeInput::EdgeRemoved, this, &NodeParamViewItemBody::EdgeChanged);
    }

    // Add keyframe control to this layout if parameter is keyframable
    if (input->is_keyframable()) {
      // Hacky but effective way to make sure this widget is always as far right as possible
      int control_column = max_col;

      ui_objects.key_control = new NodeParamViewKeyframeControl();
      ui_objects.key_control->SetInput(input);
      content_layout->addWidget(ui_objects.key_control, row_count, control_column);
      connect(ui_objects.key_control, &NodeParamViewKeyframeControl::RequestSetTime, this, &NodeParamViewItemBody::RequestSetTime);

      connect(input, &NodeInput::KeyframeEnableChanged, this, &NodeParamViewItemBody::InputKeyframeEnableChanged);
      connect(input, &NodeInput::KeyframeAdded, this, &NodeParamViewItemBody::InputAddedKeyframe);
      connect(input, &NodeInput::KeyframeRemoved, this, &NodeParamViewItemBody::KeyframeRemoved);
    }

    input_ui_map_.insert(input, ui_objects);

    // Update "connected" label
    if (input->is_connectable()) {
      UpdateUIForEdgeConnection(input);
    }

    row_count++;

    // If the row count is an array, we put an extra body widget in the next row so we skip over it here
    if (input->IsArray()) {
      row_count++;
    }
  }
}

void NodeParamViewItemBody::SetTimeTarget(Node *target)
{
  foreach (const InputUI& ui_obj, input_ui_map_) {
    // Only keyframable inputs have a key control widget
    if (ui_obj.key_control) {
      ui_obj.key_control->SetTimeTarget(target);
    }

    ui_obj.widget_bridge->SetTimeTarget(target);
  }

  foreach (NodeParamViewItemBody* sb, sub_bodies_) {
    sb->SetTimeTarget(target);
  }
}

void NodeParamViewItemBody::SetTime(const rational &time)
{
  foreach (const InputUI& ui_obj, input_ui_map_) {
    // Only keyframable inputs have a key control widget
    if (ui_obj.key_control) {
      ui_obj.key_control->SetTime(time);
    }

    ui_obj.widget_bridge->SetTime(time);
  }

  foreach (NodeParamViewItemBody* sb, sub_bodies_) {
    sb->SetTime(time);
  }
}

void NodeParamViewItemBody::Retranslate()
{
  QMap<NodeInput*, InputUI>::const_iterator i;

  for (i=input_ui_map_.begin(); i!=input_ui_map_.end(); i++) {
    i.value().main_label->setText(tr("%1:").arg(i.key()->name()));
  }

  foreach (NodeParamViewItemBody* sb, sub_bodies_) {
    sb->Retranslate();
  }
}

void NodeParamViewItemBody::SignalAllKeyframes()
{
  QMap<NodeInput*, InputUI>::const_iterator i;

  for (i=input_ui_map_.begin(); i!=input_ui_map_.end(); i++) {
    NodeInput* input = i.key();

    foreach (const NodeInput::KeyframeTrack& track, input->keyframe_tracks()) {
      foreach (NodeKeyframePtr key, track) {
        InputAddedKeyframeInternal(input, key);
      }
    }
  }

  foreach (NodeParamViewItemBody* sb, sub_bodies_) {
    sb->SignalAllKeyframes();
  }
}

void NodeParamViewItemBody::EdgeChanged()
{
  UpdateUIForEdgeConnection(static_cast<NodeInput*>(sender()));
}

void NodeParamViewItemBody::UpdateUIForEdgeConnection(NodeInput *input)
{
  // Show/hide bridge widgets
  const InputUI& ui_objects = input_ui_map_[input];

  foreach (QWidget* w, ui_objects.widget_bridge->widgets()) {
    w->setVisible(!input->is_connected());
  }

  // Show/hide connection label
  ui_objects.connected_label->setVisible(input->is_connected());
}

void NodeParamViewItemBody::InputKeyframeEnableChanged(bool e)
{
  NodeInput* input = static_cast<NodeInput*>(sender());

  foreach (const NodeInput::KeyframeTrack& track, input->keyframe_tracks()) {
    foreach (NodeKeyframePtr key, track) {
      if (e) {
        // Add a keyframe item for each keyframe
        InputAddedKeyframeInternal(input, key);
      } else {
        // Remove each keyframe item
        emit KeyframeRemoved(key);
      }
    }
  }
}

void NodeParamViewItemBody::InputAddedKeyframe(NodeKeyframePtr key)
{
  // Get NodeInput that emitted this signal
  NodeInput* input = static_cast<NodeInput*>(sender());

  InputAddedKeyframeInternal(input, key);
}

void NodeParamViewItemBody::LabelDoubleClicked()
{
  QMap<NodeInput*, InputUI>::const_iterator iterator;

  for (iterator=input_ui_map_.begin(); iterator!=input_ui_map_.end(); iterator++) {
    if (iterator.value().main_label == sender()) {
      emit InputDoubleClicked(iterator.key());
      return;
    }
  }
}

void NodeParamViewItemBody::ConnectionClicked()
{
  QMap<NodeInput*, InputUI>::const_iterator iterator;
  for (iterator=input_ui_map_.begin(); iterator!=input_ui_map_.end(); iterator++) {
    if (iterator.value().connected_label == sender()) {
      Node* connected = iterator.key()->get_connected_node();

      if (connected) {
        emit RequestSelectNode({connected});
      }

      return;
    }
  }
}

void NodeParamViewItemBody::InputAddedKeyframeInternal(NodeInput *input, NodeKeyframePtr keyframe)
{
  // Find its row in the parameters
  QLabel* lbl = input_ui_map_.value(input).main_label;

  // Find label's Y position
  QPoint lbl_center = lbl->rect().center();

  // Find global position
  lbl_center = lbl->mapToGlobal(lbl_center);

  emit KeyframeAdded(keyframe, lbl_center.y());
}

NodeParamViewItemBody::InputUI::InputUI() :
  main_label(nullptr),
  widget_bridge(nullptr),
  connected_label(nullptr),
  key_control(nullptr)
{
}

OLIVE_NAMESPACE_EXIT
