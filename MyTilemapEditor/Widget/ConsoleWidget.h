#pragma once

#include <QDockWidget>

QT_FORWARD_DECLARE_CLASS( QTextBrowser );

class ConsoleWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit ConsoleWidget( const QString& title, QWidget* parent = Q_NULLPTR );

private:

public slots:
	void print( const QString& text );
	void printError( const QString& text );
	void popupRightClickMenu( const QPoint& pos );

private:
	QTextBrowser* m_textBrowser;
};