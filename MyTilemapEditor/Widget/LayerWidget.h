#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include "Core/LayerInfo.h"
#include "Core/MapInfo.h"

QT_FORWARD_DECLARE_CLASS( QBoxLayout )
QT_FORWARD_DECLARE_CLASS( QListWidget )
QT_FORWARD_DECLARE_CLASS( QToolBar )
class LayerRowWidget;
class LayerGroup;

class LayerWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit LayerWidget( const QString& title, QWidget* parent = Q_NULLPTR );

private:
	void initialToolbar();

	void updateToolbar( bool enableNewLayer, bool enableRaise, bool enableLower, bool enableDelete );
	void moveItem( int fromItemIndex, int toItemIndex );

public slots:
	void updateToolbarStatus();
	void addNewLayer();
	void removeLayer();
	void raiseCurrentLayer();
	void lowerCurrentLayer();

	void addNewLayerGroup( MapInfo mapInfo, QList<LayerInfo> layerInfoList );
	void removeLayerGropu( int listWidgetIndex );
	void changeLayerGroup( int listWidgetIndex );

signals:
	void getTabCount( int& tabCount );

private:
	void addNewLayer( LayerInfo layerInfo );

private:
	QListWidget* m_emptyListWidget;
	QList<LayerGroup*> m_listWidgetList;
	int m_currentIndex = -1;
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
	LayerRow( QListWidget* view, LayerInfo layerInfo );

private:
	LayerRowWidget* m_layerRow;
};