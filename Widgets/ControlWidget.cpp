#include "ControlWidget.h"
#include <QPushButton>

namespace  Image{
namespace FormatTransformation {

ControlWidget::ControlWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setFixedWidth(200);

    this->color_widget(QColor(37,109,231));

}

void ControlWidget::add_pushbutton(QPushButton *_pushbutton)
{
    if(_pushbutton == NULL)return;

    this->buttons_.push_back(_pushbutton);
}

void ControlWidget::resizeEvent(QResizeEvent *)
{
    int space = 5;

    int x = 2 * space;
    int y = 2 * space;

    int width = this->width() - 4 * space;
    int height = width / 5;

    for(int i = 0; i < this->buttons_.size(); i++)
    {
        this->buttons_[i]->move(x, y);
        this->buttons_[i]->resize(width, height);

        y += /*( i + 1 ) **/ (height + space);
    }
}

void ControlWidget::color_widget(QColor _color)
{
    this->setAutoFillBackground(true);
    QPalette pal;
    pal = this->palette();
    pal.setColor(QPalette::Background, _color);
    this->setPalette(pal);
}



}}
