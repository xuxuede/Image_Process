#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
class QColor;
class QString;

namespace  Image{
namespace FormatTransformation {


class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TitleWidget(QWidget* parent = 0);

protected:
    void paintEvent(QPaintEvent* );

private:
    void color_widget(QColor _color);

private:
    QString title_text_;

};



}}



#endif //TITLEWIDGET_H
