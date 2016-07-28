QT += core gui

greaterThan(QT_MAJOR_VERSION, 4) :  QT += widgets

TEMPLATE = app

TARGET = ImageFormatTransformation

INCLUDEPATH *= ./
INCLUDEPATH *= ../

win32
{
    DEFINES += OM_STATIC_BUILD
#    win32-g++{

#    QMAKE_CXXFLAGS += -Wno-unused-parameter
#    QMAKE_CXXFLAGS += -Wno-unused-parameter
#    QMAKE_CXXFLAGS += -Wno-reorder
#    QMAKE_CXXFLAGS += -Wno-unknown-pragmas
#    QMAKE_CXXFLAGS += -Wno-deprecated-declarations
#    QMAKE_CXXFLAGS += -std=c++11

#    DEFINES += _COMPILE_GCC
#    DEFINES += USE_FREEGLUT
#    DESTDIR = $$PWD/../LS_bin/mingw/
#    LIBS += -L$$PWD/../Libraries/lib/GCL/mingw/ -lGCLGui
#    LIBS += -L$$PWD/../Libraries/lib/GCL/mingw/ -lGCLCore
#    LIBS += -L$$PWD/../Libraries/lib/OpenMesh/mingw/ -lOpenMeshCore
#    LIBS += -L$$PWD/../Libraries/lib/GL/mingw/ -lfreeglut
#    }

    LIBS += -lopengl32 -lglu32

    !win32-g++
    {
        DEFINES += _USE_MATH_DEFINES
        win32-msvc2010{
            DESTDIR = $$PWD/bin/vs2010/


        }
#        win32-msvc2012{
#            DESTDIR = $$PWD/../LS_bin/vs2012/
#        }
#        win32-msvc2013{
#            DESTDIR = $$PWD/../LS_bin/vs2013/
#            LIBS += -L$$PWD/../Libraries/lib/GCL/vs2013/ -lGCLGui
#            LIBS += -L$$PWD/../Libraries/lib/GCL/vs2013/ -lGCLCore
#            LIBS += -L$$PWD/../Libraries/lib/OpenMesh/vs2013/ -lOpenMeshCore
#        }

        DEPENDPATH += $$PWD/../Libraries/bin/GL/msvc/
    }
}

SOURCES += \
    Algorithms/BitTransformation.cpp \
    Algorithms/BmpToJpg.cpp \
    Widgets/ControlWidget.cpp \
    Widgets/MainWidget.cpp \
    Widgets/PictureShowWidget.cpp \
    Widgets/TitleWidget.cpp \
    main.cpp


HEADERS += \
    Algorithms/BitTransformation.h \
    Algorithms/BmpToJpg.h \
    Widgets/ControlWidget.h \
    Widgets/MainWidget.h \
    Widgets/PictureShowWidget.h \
    Widgets/TitleWidget.h


RESOURCES += \
    images.qrc
    


TRANSLATIONS += \
