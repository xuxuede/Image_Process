#include "MainWidget.h"
#include <QColor>
#include <QPalette>
#include "TitleWidget.h"
#include <iostream>
#include <QResizeEvent>
#include <QSplitter>
#include "ControlWidget.h"
#include "PictureShowWidget.h"
#include <QPushButton>
#include <QByteArray>
#include <QString>
#include <QFileDialog>
#include "./Algorithms/BmpToJpg.h"
#include "./Algorithms/BitTransformation.h"

namespace  Image{
namespace FormatTransformation {

MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent),bmp_filename_("")
{
    this->resize(1000, 600);

    this->color_widget(QColor(170,253,234));

    this->create_widget();
}

MainWidget::~MainWidget()
{

}

void MainWidget::slot_open_image()
{
    QString filename = QFileDialog::getOpenFileName(0, QString(tr("Open Image")), QString("../../Images"), QString("images(*.bmp)"));
    emit signal_clear_screen();

    if(filename.isNull())return;

    this->bmp_filename_ = filename;

    emit signal_show_image(0, filename);

}

void MainWidget::slot_bmp_to_jpg()
{

    char* BMP_filename;
    char JPG_filename[100];

    QByteArray ba = this->bmp_filename_.toLatin1();
    BMP_filename = ba.data();

    int len_filename;
    strcpy(JPG_filename,BMP_filename);
    len_filename = strlen(BMP_filename);
    strcpy(JPG_filename+(len_filename-3),"jpg");

    BmpToJpg bmpToJpg;

    bmpToJpg.do_tranformation(JPG_filename, BMP_filename);

    emit signal_show_image(1, JPG_filename);
}

void MainWidget::slot_24bit_8bit()
{
    char* BMP_filename;
    char* BMP_filename_new;
    QString tmp_str = this->bmp_filename_;
    tmp_str.insert(tmp_str.size() - 4, "_8bit");


    BMP_filename = this->bmp_filename_.toLatin1().data();
    BMP_filename_new = tmp_str.toLatin1().data();

//    std::cout<<tmp_str.toStdString()<<std::endl;

    BitTransformation bit_trans;
    bit_trans.do_tranformation(BMP_filename, BMP_filename_new);

    emit signal_show_image(1, BMP_filename_new);

}

void MainWidget::color_widget(QColor _color)
{
    this->setAutoFillBackground(true);
    QPalette pal;
    pal = this->palette();
    pal.setColor(QPalette::Background, _color);
    this->setPalette(pal);
}

void MainWidget::resizeEvent(QResizeEvent *)
{
    this->title_widget_->move(0, 0);
    this->title_widget_->resize(this->width(), this->height() / 4);

    this->splitter->move(0, this->height() / 4);
    this->splitter->resize(this->width(), this->height() * 3 / 4);
}

void MainWidget::create_widget()
{   
    this->title_widget_ = new TitleWidget(this);
    this->control_widget_ = new ControlWidget(this);

    this->button_openfile_ = new QPushButton(tr("Open Image"), this->control_widget_);
    connect(this->button_openfile_, SIGNAL(clicked()), this, SLOT(slot_open_image()));

    this->button_bmp_to_jpg_ = new QPushButton(tr("Bmp To Jpg"), this->control_widget_);
    connect(this->button_bmp_to_jpg_, SIGNAL(clicked()), this, SLOT(slot_bmp_to_jpg()));

    this->button_24bit_to_8bit_ = new QPushButton(tr("24bit To 8bit"), this->control_widget_);
    connect(this->button_24bit_to_8bit_, SIGNAL(clicked()), this, SLOT(slot_24bit_8bit()));

    this->control_widget_->add_pushbutton(this->button_openfile_);
    this->control_widget_->add_pushbutton(this->button_bmp_to_jpg_);
    this->control_widget_->add_pushbutton(this->button_24bit_to_8bit_);

    this->pic_show_widget_input_ = new PictureShowWidget(0, this);
    connect(this, SIGNAL(signal_show_image(int,QString)), this->pic_show_widget_input_, SLOT(slot_show_image(int,QString)));
    connect(this, SIGNAL(signal_clear_screen()), this->pic_show_widget_input_, SLOT(slot_clear_screen()));

    this->pic_show_widget_output_ = new PictureShowWidget(1, this);
    connect(this, SIGNAL(signal_show_image(int,QString)), this->pic_show_widget_output_, SLOT(slot_show_image(int,QString)));
    connect(this, SIGNAL(signal_clear_screen()), this->pic_show_widget_output_, SLOT(slot_clear_screen()));

    this->splitter = new QSplitter(this);

    this->splitter->addWidget(this->pic_show_widget_input_);
    this->splitter->addWidget(this->pic_show_widget_output_);
    this->splitter->addWidget(this->control_widget_);


}






}}
