#pragma once

#include <QDockWidget>
#include <QTreeWidget>

QT_FORWARD_DECLARE_CLASS( QTreeView )
QT_FORWARD_DECLARE_CLASS( QBoxLayout )

class LayerWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit LayerWidget( const QString& title, QWidget* parent = Q_NULLPTR );

private:
	void initialToolbar();

public slots:

private:
	QTreeWidget* m_treeWidget;
	QBoxLayout* m_layout;
};