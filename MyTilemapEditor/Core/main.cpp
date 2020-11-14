#include "Widget/MainWindow.h"
#include <QtWidgets/QApplication>
#include <QObject>
#include "ThirdParty/framelesswindowDarkStyle/framelesswindow/framelesswindow.h"
#include "ThirdParty/framelesswindowDarkStyle/DarkStyle.h"

int main(int argc, char *argv[])
{
	QApplication a( argc, argv );
    
    // style our application with custom dark style
    QApplication::setStyle( new DarkStyle );
	QApplication::setPalette( QApplication::style()->standardPalette() );

    MainWindow* w = new MainWindow;
    w->show();

	// create frameless window (and set windowState or title)
// 	FramelessWindow* framelessWindow = new FramelessWindow;
// 	framelessWindow->setWindowTitle("MyTileEditor");
// 	framelessWindow->setContent( w );
// 	framelessWindow->show();
//  	QObject::connect( w, &MainWindow::quit, framelessWindow, &FramelessWindow::on_closeButton_clicked );

    return a.exec();
}
