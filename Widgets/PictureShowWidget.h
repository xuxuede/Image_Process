#ifndef PICTURESHOWWIDGET_H
#define PICTURESHOWWIDGET_H
#include <QWidget>

class QImage;
class QPainter;

namespace  Image{
namespace FormatTransformation {

class PictureShowWidget : public QWidget
{
    Q_OBJECT
public:
    PictureShowWidget(int _id, QWidget* parent = 0);

public slots:
    void slot_show_image(int _id, const QString& _image_name);
    void slot_clear_screen();

protected:
    void paintEvent(QPaintEvent* );

    void paint_widget(QPainter& _painter);

private:
    QImage* image_;
    int id_;
    bool is_drawing_;

    QString title_text_;
};



}}


#endif //PICTURESHOWWIDGET_H
