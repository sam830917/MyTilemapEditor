#pragma once

#include "MapInfo.h"
#include "TileInfo.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QList>
#include <QSet>
#include <QUndoStack>

class Tile;
class WorkspaceWidget;

class MapScene : public QGraphicsScene
{
	friend class WorkspaceWidget;
	friend class DrawCommand;

public:
	MapScene( MapInfo mapInfo, WorkspaceWidget* parent = Q_NULLPTR );

	MapInfo getMapInfo() { return m_mapInfo; }

	void editMapOnPoint( const QPointF& point );

	void paintMap( int index, TileInfo tileInfo );
	void paintMap( int index );
	void paintMap( QSize coord );
	void eraseMap( int index );
	void eraseMap( QSize coord );

protected:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	bool m_isSaved = true;
	QGraphicsView* m_view;
	WorkspaceWidget* m_parentWidget;
	MapInfo m_mapInfo;
	QList<Tile*> m_tileList;

	QList<TileInfo> m_beforeDrawTileInfo;
	QUndoStack* m_undoStack;
};

class Tile : public QGraphicsRectItem
{
	friend class MapScene;
	friend class DrawCommand;

public:
	Tile( MapScene* scene, QGraphicsItem* parent = Q_NULLPTR );

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

	void setTileInfo( TileInfo tileInfo ) { m_tileInfo = tileInfo; }
	TileInfo getTileInfo() { return m_tileInfo; }
protected:

private:
	TileInfo m_tileInfo;
	MapScene* m_mapScene;
};