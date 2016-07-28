#ifndef MAINWIDGET_H
#define MAINWIDGET_H
#include <QWidget>

class QColor;
class QSplitter;
class QPushButton;
class QString;

namespace  Image{
namespace FormatTransformation {

class TitleWidget;
class ControlWidget;
class PictureShowWidget;

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

signals:
    void signal_show_image(int _id, const QString& _image_name);//信号与槽
    void signal_clear_screen();

public slots:
    void slot_open_image();
    void slot_bmp_to_jpg();
    void slot_24bit_8bit();

private:
    void color_widget(QColor _color);
    void resizeEvent(QResizeEvent* );

    void create_widget();
private:
    TitleWidget* title_widget_;
    ControlWidget* control_widget_;
    PictureShowWidget* pic_show_widget_input_;
    PictureShowWidget* pic_show_widget_output_;

    QSplitter* splitter;

    QPushButton* button_openfile_;
    QPushButton* button_bmp_to_jpg_;
    QPushButton* button_24bit_to_8bit_;

    QString bmp_filename_;

};



}
}




#endif//MAINWIDGET_H
