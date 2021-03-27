#pragma once

#include "Core/MapScene.h"
#include "Core/MapInfo.h"
#include "Core/LayerInfo.h"
#include "Utils/ProjectCommon.h"
#include "Brush/BrushCommon.h"
#include <QWidget>
#include <QTabWidget>
#include <QList>
#include <QMap>

QT_FORWARD_DECLARE_CLASS( QPushButton );

class WorkspaceWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WorkspaceWidget( QWidget* parent = Q_NULLPTR );

	void disableTabWidget( bool disable ) const;
	bool isReadyToClose();
	QStringList getOpeningMapFilePath() const;

	eDrawTool getCurrentDrawTool() { return m_drawTool; }

protected:
	virtual bool eventFilter( QObject* obj, QEvent* event );

private:
	void saveLayerToFile( QList<LayerInfo>& layerInfoList, int tabIndex, XmlElement& mapLayersEle, XmlDocument& mapDoc );
	void saveTileToFile( QList<const Tileset*>& tilesetList, int tabIndex, XmlElement& mapRoot, XmlDocument& mapDoc );

public slots:
	void popupRightClickMenu( const QPoint& pos );
	void resetTabWidgetVisibility();
	void markCurrentSceneForModified();
	void addMap();
	void insertMap( MapInfo mapInfo, QList<LayerInfo> layerInfoList );
	void closeTab( int index );
	void setDrawTool( eDrawTool drawTool );
	void changeTab( int index );
	void changeMapScale( const QString& text );
	void nextTab();
	void closeCurrentTab();

	void saveCurrentMap();
	void saveAllMaps();
	void saveMap( int tabIndex );
	void getTabCount( int& tabCount );

	void addNewLayerIntoMap( int index, const QString& name );
	void addNewMarkerLayerIntoMap( int index, const QString& name );
	void changeLayerOrder( int indexA, int indexB );
	void deleteLayerFromIndex( int index );
	void setLayerLock( int index, bool isLock );
	void setLayerVisible( int index, bool isVisible );
	void setLayerColor( int index,  const QColor& color );
	void setLayerName( int index, const QString& name );
	void changeLayerFocus();

	void eraseSelectedTilesInCurrentLayer();
	void selecteAllTilesInCurrentLayer();
	void copySelectedTile();
	void cutSelectedTile();
	void pasteCopiedTile();
	void closeAllTab();
	void exportXMLFile();
	void exportPNGFile();

signals:
	void updateRedo( QAction* action );
	void updateUndo( QAction* action );
	void disableShortcut( bool isDisable );
	void tabFocusChanged( int index );
	void closeTabSuccessfully( int index );
	void getLayerIndex( int& index );
	void getLayerGroupInfoList( int index, QList<LayerInfo>& layerInfoList );
	void movedLayerOrder( int fromItemIndex, int toItemIndex );
	void addedNewLayer( int index, const QString& name, eLayerType type );
	void addedNewLayerWithInfo( int index, LayerInfo layerInfo );
	void deletedLayer( int index );
	void renamedLayer( int index, const QString& name );

	void showProperties( const QMap<QString, QString>& informationMap );
	void addNewLayerGroup( MapInfo mapInfo, QList<LayerInfo> layerInfoList );
	void getPaintMapModified( QList<TileModified>& modifiredList, const QPoint& point, eDrawTool tool );
	void isDefalutBrush( bool& isDefalut );
	void changeDrawTool( eDrawTool tool );

public:
	QPushButton* m_newProjectButton = Q_NULLPTR;
	QPushButton* m_openProjectButton = Q_NULLPTR;
	QPushButton* m_newMapButton = Q_NULLPTR;
	QPushButton* m_newTilesetButton = Q_NULLPTR;
	QTabWidget* m_mapTabWidget = Q_NULLPTR;
	QList<MapScene*> m_mapSceneList;
	eDrawTool m_drawTool = eDrawTool::BRUSH;
	QCursor m_currentCursor = Qt::ArrowCursor;
};