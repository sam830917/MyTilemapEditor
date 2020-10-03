#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QListWidgetItem>

QT_FORWARD_DECLARE_CLASS( QBoxLayout )
QT_FORWARD_DECLARE_CLASS( QListWidget )
QT_FORWARD_DECLARE_CLASS( QToolBar )
class LayerRowWidget;

class LayerWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit LayerWidget( const QString& title, QWidget* parent = Q_NULLPTR );

private:
	void initialToolbar();

	void updateToolbar( bool enableNewLayer, bool enableRaise, bool enableLower, bool enableDelete );
	void updateToolbarStatus();
	void moveItem( int fromItemIndex, int toItemIndex );
public slots:
	void addNewLayer();
	void removeLayer();
	void raiseCurrentLayer();
	void lowerCurrentLayer();

private:
	QListWidget* m_listWidget;
	QBoxLayout* m_layout;

	QToolBar* m_toolbar;
	QAction* m_newLayerAction;
	QAction* m_raiseAction;
	QAction* m_lowerAction;
	QAction* m_deleteAction;
};

class LayerRow : public QListWidgetItem
{
	friend class LayerWidget;

public:
	LayerRow( QListWidget* view );

private:
	LayerRowWidget* m_layerRow;
};