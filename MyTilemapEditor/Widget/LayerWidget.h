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
QT_FORWARD_DECLARE_CLASS( QPushButton )
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

public slots:
	void updateToolbarStatus();
	void addNewLayer();
	void addNewMarkerLayer();
	void implementAddNewLayer( int index, const QString& name, eLayerType type );
	void implementAddNewLayerWithInfo( int index, LayerInfo layerInfo );
	void implementRenameLayer( int index, const QString& name );
	void removeLayerFromIndex( int index );
	void removeLayer();
	void raiseCurrentLayer();
	void lowerCurrentLayer();

	void addNewLayerGroup( MapInfo mapInfo, QList<LayerInfo> layerInfoList );
	void removeLayerGropu( int listWidgetIndex );
	void switchLayerGroup( int listWidgetIndex );
	void getLayerIndex( int& index );
	void getLayerGroupInfoList( int index, QList<LayerInfo>& layerInfoList );
	void moveItem( int fromItemIndex, int toItemIndex );

signals:
	void getTabCount( int& tabCount );
	void addedNewLayerFromIndex( int index, const QString& name );
	void addedNewMarkerLayerFromIndex( int index, const QString& name );
	void movedLayerGroup( int fromItemIndex, int toItemIndex );
	void deletedLayer( int index );
	void setLayerIsLock( int index, bool isLock );
	void setLayerIsVisible( int index, bool isVisible );
	void setLayerName( int index, const QString& name );
	void modifiedCurrentScene();
	void changeLayerFocus();
	void changeColor( int index,  const QColor& color );

private:
	void addNewLayer( LayerInfo layerInfo );

private:
	QListWidget* m_emptyListWidget;
	QList<LayerGroup*> m_listWidgetList;
	int m_currentIndex = -1;
	QBoxLayout* m_layout;

	QToolBar* m_toolbar;
	QAction* m_newLayerAction;
	QAction* m_newMarkerLayerAction;
	QAction* m_raiseAction;
	QAction* m_lowerAction;
	QAction* m_deleteAction;
};

class LayerRow : public QListWidgetItem
{
	friend class LayerWidget;
	friend class LayerRowWidget;

public:
	LayerRow( LayerGroup* view, int insertIndex, bool isMarkerLayer = false );
	LayerRow( LayerGroup* view, LayerInfo layerInfo );

	void setLayerInfo( const LayerInfo& layerInfo );

private:
	LayerRowWidget* m_layerRowWidget;
	LayerGroup* m_layerGroup;
	int m_index = -1;
};

class LayerRowWidget : public QWidget
{
	friend class LayerItem;
	friend class LayerWidget;
	friend class LayerRow;
	Q_OBJECT

public:
	LayerRowWidget( const QString& name, LayerRow* layerRow, bool lockChecked = false, bool visibleChecked = true, bool isMarkerLayer = false, QWidget* parent = Q_NULLPTR );

	QString getName() const;

public slots:
	void setIsLock();
	void setIsVisible();
	void setColor();

protected:
	virtual bool eventFilter( QObject* obj, QEvent* event );

private:
	QLineEdit* m_lineEdit;
	QLabel* m_label;
	QStackedWidget* m_stackedWidget;
	QCheckBox* m_lockBtn;
	QCheckBox* m_visibleBtn;
	LayerRow* m_layerRow;

	bool m_isMarkerLayer = false;
	QColor m_markerColor;
	QPushButton* m_markerColorButton;
};

class LayerGroup : public QListWidget
{
	friend class LayerWidget;
	friend class LayerRowWidget;
	Q_OBJECT

public:
	LayerGroup( MapInfo mapInfo, LayerWidget* parent = Q_NULLPTR );

private:
	MapInfo m_mapInfo;
	LayerWidget* m_layerWidget;
};