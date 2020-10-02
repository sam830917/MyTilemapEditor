#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

QT_FORWARD_DECLARE_CLASS( QTreeView )
QT_FORWARD_DECLARE_CLASS( QBoxLayout )
QT_FORWARD_DECLARE_CLASS( QToolBar )

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
	QToolBar* m_toolbar;
};

class LayerItem : public QTreeWidgetItem
{

public:
	LayerItem( QTreeWidget* view );

private:
};