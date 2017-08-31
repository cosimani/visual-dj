#include <QApplication>
#include "scene.hpp"

int main( int argc, char **argv )
{
    QApplication application( argc, argv );

    Scene scene;
    scene.showFullScreen();
//    scene.show();
//    scene.showMaximized();

    return application.exec();
}

