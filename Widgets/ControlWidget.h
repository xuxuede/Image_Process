#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H
#include <QWidget>

class QPushButton;

namespace  Image{
namespace FormatTransformation {

class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(QWidget* parent = 0);

    void add_pushbutton(QPushButton* _pushbutton);

protected:
    void resizeEvent(QResizeEvent* );

private:
    void color_widget(QColor _color);

private:
    QList<QPushButton* > buttons_;

};



}}


#endif //CONTROWIDGET_H
