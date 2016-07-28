#include "PictureShowWidget.h"
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QPen>
#include <QFont>

namespace  Image{
namespace FormatTransformation {

PictureShowWidget::PictureShowWidget(int _id, QWidget *parent)
    :QWidget(parent), id_(_id),is_drawing_(false)
{
    this->image_ = new QImage();

    if(this->id_ == 0)
    {
        this->title_text_ = tr("InPut Image");
    }
    else if(this->id_ == 1)
    {
        this->title_text_ = tr("Output Image");
    }
}

void PictureShowWidget::slot_show_image(int _id, const QString &_image_name)
{
    if(this->id_ != _id)   return;

    this->image_->load(_image_name);

    if(!this->image_->isNull())
    {
        this->is_drawing_ = true;
        update();
    }
}

void PictureShowWidget::slot_clear_screen()
{
    this->is_drawing_ = false;
    update();
}

void PictureShowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    int space = 10;
    int x = space;
    int y = 10 * space;


    QFont font;
    font.setPixelSize(30);
    font.setFamily("Microsoft Yahei");
    painter.setFont(font);
    painter.drawText(QRect(space, space, this->width() - 2 *space, 8 * space ), this->title_text_);


    this->paint_widget(painter);

    if(is_drawing_)
    {
        painter.drawImage(QRect(x, y, this->width() - 2 * space, this->height() - y - space), *this->image_);
    }
}

void PictureShowWidget::paint_widget(QPainter &_painter)
{
    int space = 10;

    _painter.setPen(QPen(Qt::black, space));
    _painter.drawRect(0,0, this->width(), this->height());
    _painter.setPen(Qt::NoPen);

    _painter.setBrush(QBrush(Qt::white));
    int x = space;
    int y = 10 * space;
    _painter.drawRect(x, y, this->width() - 2 * space, this->height() - y - space);
    _painter.setBrush(Qt::NoBrush);
}




}}
