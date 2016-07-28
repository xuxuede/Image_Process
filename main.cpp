#include <QApplication>
#include "Algorithms/BmpToJpg.h"
#include "Widgets/MainWidget.h"
#include <QTranslator>
#include <iostream>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

//    QTranslator translator;
//    translator.load(":/translate/ImageFormatTransformation.qm");

//    app.installTranslator(&translator);

    Image::FormatTransformation::MainWidget mainwidget;//生成对象
    mainwidget.show();

    app.exec();

    return 0;
}

