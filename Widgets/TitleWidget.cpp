#include "TitleWidget.h"
#include <QPainter>
#include <QFont>
#include <QPixmap>
#include <QColor>

namespace  Image{
namespace FormatTransformation {

TitleWidget::TitleWidget(QWidget *parent)
    :QWidget(parent)
{
    this->color_widget(QColor(255, 255, 255));

    this->title_text_ = tr("Image Format Transformation");
}

void TitleWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pic1(":/icons/Resources/flower1.jpg");
    QPixmap pic2(":/icons/Resources/flower2.jpg");

    painter.drawPixmap(0, 0, this->width() / 5, this->height(), pic1);
    painter.drawPixmap(this->width() * 4 / 5, 0, this->width() / 5, this->height(), pic2);

    QFont font;
    font.setFamily("Microsoft Yahei");
    font.setPixelSize(40);
    painter.setFont(font);
//    painter.drawText(this->width() / 5 + 10, this->height() / 10, this->width() * 3 / 5 - 20, this->height() * 8 / 10, this->title_text_);
    painter.drawText(this->width() / 5 + 10, this->height() / 2, this->title_text_);

}


void TitleWidget::color_widget(QColor _color)
{
    this->setAutoFillBackground(true);
    QPalette pal;
    pal = this->palette();
    pal.setColor(QPalette::Background, _color);
    this->setPalette(pal);
}






}}
