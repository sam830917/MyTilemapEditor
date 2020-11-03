#pragma once

#include <QObject>

class JSConsole : public QObject
{
	Q_OBJECT
public:
	explicit JSConsole( QObject* parent = 0 );

signals:

public slots:
	void log( QString msg );
};