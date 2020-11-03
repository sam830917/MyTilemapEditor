#include "Brush/Jsconsole.h"
#include "Utils/ProjectCommon.h"
#include <QDebug>

JSConsole::JSConsole( QObject* parent ):
	QObject( parent )
{
}

void JSConsole::log( QString msg )
{
	debugPrint( msg );
}