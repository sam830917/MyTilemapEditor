#pragma once

#include "MapInfo.h"
#include "UndoCommand.h"
#include "TileInfo.h"
#include "Core/Layer.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QList>
#include <QSet>
#include <QUndoStack>

class WorkspaceWidget;
class SelectMask;
struct TileModified;

class MapView : public QGraphicsView
{
public:
	MapView( MapScene* scene, QWidget* parent = Q_NULLPTR );

	void updateCursor();

protected:
	virtual void wheelEvent( QWheelEvent* event );
	virtual void enterEvent(QEvent* event);

public:
	MapScene* m_mapScene = nullptr;
};

class MapScene : public QGraphicsScene
{
	friend class MapView;
	friend class WorkspaceWidget;
	friend class DrawCommand;
	friend class LayerMoveCommand;
	friend class LayerAddCommand;
	friend class LayerDeleteCommand;
	friend class LayerRenameCommand;
	friend class LayerColorChangeCommand;
	friend class DrawMarkerCommand;
	friend class TileLayer;
	friend class MarkerLayer;
	friend class SelectMask;
	friend class BrushHelper;

public:
	MapScene( MapInfo mapInfo, WorkspaceWidget* parent );
	MapScene( MapInfo mapInfo );
	~MapScene();

	MapInfo getMapInfo() const { return m_mapInfo; }
	TileInfo getTileInfo( int tileIndex, int layerIndex ) const;
	bool getIsMarked( int tileIndex, int layerIndex ) const;

	void editMapOnPoint( const QPointF& point );
	QList<QPoint> editMapByFloodFill( int layerIndex, const QPoint& coord );
	void selectTilesByFloodFill( int layerIndex, const QPoint& coord );

	void paintMap( int index, TileInfo tileInfo, int layerIndex );
	void paintMap( const QPoint& coord, TileInfo tileInfo, int layerIndex );
	void paintMap( int index, TileInfo tileInfo );
	void paintMap( int index );
	void paintMap( QSize coord );
	void paintMap( QPoint coord, TileInfo tileInfo );
	void eraseMap( int tileIndex, int layerIndex );
	void eraseMap( const QPoint& coord, int layerIndex );
	void eraseMap( int index );
	void eraseMap( QPoint coord );

	TileLayer* addNewLayer( int zValue );
	MarkerLayer* addNewMarkerLayer( int zValue );

	void setIsShowSelection( bool isShow );
	void updateSelection();
	void eraseSelectedTiles();
	void selecteAllTiles();
	QList<TileModified> getCopiedTiles() const;
	void pasteTilesOnCoord( const QPoint& coord, const QList<TileModified>& copiedTileList );

private:
	int getCurrentLayerIndex() const;
	void paintMap( const QMap<int, TileInfo>& tileInfoMap, int layerIndex );
	void showTileProperties( const QPointF& mousePos );
	void showSelectedTileProperties();
	void paintTileByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, QList<QPoint>& readyToPaintTileIndexes );
	void selectTilesByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, const TileInfo& newTileInfo );

protected:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	bool m_isSaved = true;
	MapView* m_view = Q_NULLPTR;
	WorkspaceWidget* m_parentWidget;
	MapInfo m_mapInfo;
	QList<Layer*> m_layers;

	QSet<TileModified> m_oldTileModifiedList;
	QSet<TileMarkerModified> m_oldTileMarkerModifiedList;
	QUndoStack* m_undoStack = Q_NULLPTR;

	// Select tool
	bool m_showSelection = false;
	bool m_isSelectedMoreThanOneTile = false;
	int m_currentSelectedIndex = -1;
	QList<SelectMask*> m_selectedTileItemList;
	QPointF m_startPos;
	QPoint m_selectedMinCoord;
	QPoint m_selectedMaxCoord;
	QPoint m_lastPaintCoord;
};