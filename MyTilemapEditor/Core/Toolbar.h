#pragma once

#include <QToolBar>

QT_FORWARD_DECLARE_CLASS( QAction )
QT_FORWARD_DECLARE_CLASS( QActionGroup )
QT_FORWARD_DECLARE_CLASS( QMenu )
QT_FORWARD_DECLARE_CLASS( QSpinBox )

class ToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit ToolBar( const QString& title, QWidget* parent );

	QAction* addNewAction( const QIcon &icon, const QString &text );

private slots:

private:
};