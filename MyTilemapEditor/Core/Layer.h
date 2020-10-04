#pragma once

#include "Core/TileInfo.h"
#include <QGraphicsRectItem>
#include <QList>
class MapScene;
class Tile;

class Layer
{
	friend class MapScene;
	friend class Tile;

public:
	Layer( MapScene* mapScene, int zValue );
	~Layer();

	void setOrder( int value );

public slots:
	void setIsLock( bool islock ) { m_isLock = islock; }

private:
	MapScene* m_mapScene;
	QList<Tile*> m_tileList;
	bool m_isLock = false;
	bool m_isVisible = true;
};

class Tile : public QGraphicsRectItem
{
	friend class MapScene;
	friend class DrawCommand;

public:
	Tile( MapScene* scene, Layer* layer, QGraphicsItem* parent = Q_NULLPTR );

	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;

	void setTileInfo( TileInfo tileInfo ) { m_tileInfo = tileInfo; }
	TileInfo getTileInfo() { return m_tileInfo; }

protected:

private:
	TileInfo m_tileInfo;
	MapScene* m_mapScene;
	Layer* m_layer;
};