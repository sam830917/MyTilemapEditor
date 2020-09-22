#include "Toolbar.h"
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QLabel>
#include <QToolTip>

ToolBar::ToolBar( const QString& title, QWidget* parent )
	: QToolBar( parent )
{
	setWindowTitle( title );
	setObjectName( title );
}

QAction* ToolBar::addNewAction( const QIcon& icon, const QString& text )
{
	QAction* action = new QAction( icon, text, this );
	addAction(action);
	return action;
}