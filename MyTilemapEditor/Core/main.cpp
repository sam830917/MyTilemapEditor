#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "../ThirdParty/framelesswindowDarkStyle/framelesswindow/framelesswindow.h"
#include "../ThirdParty/framelesswindowDarkStyle/DarkStyle.h"

int main(int argc, char *argv[])
{
	QApplication a( argc, argv );
    
    // style our application with custom dark style
	a.setStyle( new DarkStyle );
	// create frameless window (and set windowState or title)
	FramelessWindow framelessWindow;
	framelessWindow.setWindowTitle("MyTileEditor");

    MainWindow* w = new MainWindow;
	framelessWindow.setContent( w );
	framelessWindow.show();

    return a.exec();
}
