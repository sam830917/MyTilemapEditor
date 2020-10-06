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

public:
	MapScene( MapInfo mapInfo, WorkspaceWidget* parent = Q_NULLPTR );

	MapInfo getMapInfo() { return m_mapInfo; }

	void editMapOnPoint( const QPointF& point );

	void paintMap( int index, TileInfo tileInfo, int layerIndex );
	void paintMap( int index, TileInfo tileInfo );
	void paintMap( int index );
	void paintMap( QSize coord );
	void eraseMap( int index );
	void eraseMap( QSize coord );

	Layer* addNewLayer( int zValue );

	void setIsShowSelection( bool isShow );
	void updateSelection();
	void eraseSelectedTiles();
	void selecteAllTiles();

private:
	void paintMap( const QMap<int, TileInfo>& tileInfoMap, int layerIndex );
	void showTileProperties( const QPointF& mousePos );
	void showSelectedTileProperties();

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
};