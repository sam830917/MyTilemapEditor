#pragma once

#include "MapInfo.h"
#include "TileInfo.h"
#include "Core/Layer.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QList>
#include <QUndoStack>

class WorkspaceWidget;
class SelectMask;

class MapView : public QGraphicsView
{
public:
	MapView( WorkspaceWidget* parent = Q_NULLPTR );
	MapView( QGraphicsScene* scene, QWidget* parent = Q_NULLPTR );

protected:
	virtual void wheelEvent( QWheelEvent* event );
};

class MapScene : public QGraphicsScene
{
	friend class WorkspaceWidget;
	friend class DrawCommand;
	friend class LayerMoveCommand;
	friend class LayerAddCommand;
	friend class LayerDeleteCommand;
	friend class LayerRenameCommand;
	friend class Layer;
	friend class SelectMask;
	friend class BrushHelper;

public:
	MapScene( MapInfo mapInfo, WorkspaceWidget* parent = Q_NULLPTR );

	MapInfo getMapInfo() { return m_mapInfo; }

	void editMapOnPoint( const QPointF& point );
	QList<QPoint> editMapByFloodFill( int layerIndex, const QPoint& coord );
	void selectTilesByFloodFill( int layerIndex, const QPoint& coord );

	void paintMap( int index, TileInfo tileInfo, int layerIndex );
	void paintMap( int index, TileInfo tileInfo );
	void paintMap( int index );
	void paintMap( QSize coord );
	void paintMap( QPoint coord, TileInfo tileInfo );
	void eraseMap( int index );
	void eraseMap( QPoint coord );

	Layer* addNewLayer( int zValue );

	void setIsShowSelection( bool isShow );
	void updateSelection();
	void eraseSelectedTiles();
	void selecteAllTiles();

private:
	int getCurrentLayerIndex();
	void paintMap( const QMap<int, TileInfo>& tileInfoMap, int layerIndex );
	void showTileProperties( const QPointF& mousePos );
	void showSelectedTileProperties();
	void paintTileByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, const TileInfo& newTileInfo, QList<QPoint>& readyToPaintTileIndexes );
	void selectTilesByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, const TileInfo& newTileInfo );

protected:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	bool m_isSaved = true;
	MapView* m_view;
	WorkspaceWidget* m_parentWidget;
	MapInfo m_mapInfo;
	QList<Layer*> m_layers;

	QList<TileInfo> m_beforeDrawTileInfo;
	QUndoStack* m_undoStack;

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