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
QT_FORWARD_DECLARE_CLASS( QLabel )
QT_FORWARD_DECLARE_CLASS( QStackedWidget )
QT_FORWARD_DECLARE_CLASS( QCheckBox )
QT_FORWARD_DECLARE_CLASS( QLineEdit )
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
	void getLayerIndex( int& index );
// 	void setIsLock( int index, bool isLock );

signals:
	void getTabCount( int& tabCount );
	void addedNewLayerGroup( int index );
	void movedLayerGroup( int fromItemIndex, int toItemIndex );
	void deletedLayerGroup( int index );

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
	friend class LayerRowWidget;

public:
	LayerRow( LayerGroup* view );
	LayerRow( LayerGroup* view, LayerInfo layerInfo );

private:
	LayerRowWidget* m_layerRowWidget;
	LayerGroup* m_layerGroup;
};

class LayerRowWidget : public QWidget
{
	friend class LayerItem;
	friend class LayerWidget;
	friend class LayerRow;
	Q_OBJECT

public:
	LayerRowWidget( const QString& name, LayerRow* layerRow, bool lockChecked = false, bool visibleChecked = true, QWidget* parent = Q_NULLPTR );

	QString getName() const;

public slots:
	void setIsLock();

protected:
	virtual bool eventFilter( QObject* obj, QEvent* event );

private:
	QLineEdit* m_lineEdit;
	QLabel* m_label;
	QStackedWidget* m_stackedWidget;
	QCheckBox* m_lockBtn;
	QCheckBox* m_visibleBtn;
	LayerRow* m_layerRow;
};

class LayerGroup : public QListWidget
{
	friend class LayerWidget;
	Q_OBJECT

public:
	LayerGroup( MapInfo mapInfo, QWidget* parent = Q_NULLPTR );

private:
	MapInfo m_mapInfo;
};