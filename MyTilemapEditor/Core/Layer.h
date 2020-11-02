#pragma once

#include "Core/TileInfo.h"
#include "Core/LayerInfo.h"
#include <QGraphicsRectItem>
#include <QList>
class MapScene;
class Tile;

class Layer
{
	friend class WorkspaceWidget;
	friend class MapScene;
	friend class LayerAddCommand;
	friend class LayerDeleteCommand;
	friend class Tile;
	friend class BrushHelper;

public:
	Layer( MapScene* mapScene, int zValue );
	~Layer();

	void setOrder( int value );

	void setLayerInfo( const LayerInfo& layerInfo ) { m_layerInfo = layerInfo; }
	void setName( const QString& name ) { m_layerInfo.setName( name ); }
	void setIsLock( bool islock ) { m_layerInfo.setIsLock( islock ); }
	void setIsVisible( bool isVisible ) { m_layerInfo.setIsVisible( isVisible ); }

	LayerInfo getLayerInfo() const { return m_layerInfo; }

private:
	MapScene* m_mapScene;
	QList<Tile*> m_tileList;
	LayerInfo m_layerInfo;
};

class Tile : public QGraphicsRectItem
{
	friend class MapScene;
	friend class DrawCommand;
	friend class LayerDeleteCommand;

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