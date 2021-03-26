#pragma once

#include "Core/TileInfo.h"
#include "Core/LayerInfo.h"
#include <QGraphicsRectItem>
#include <QList>
class MapScene;
class Tile;

class Layer
{
public:
	Layer( MapScene* mapScene, int zValue );
	virtual ~Layer();

	void setLayerInfo( const LayerInfo& layerInfo ) { m_layerInfo = layerInfo; }
	void setName( const QString& name ) { m_layerInfo.setName( name ); }
	void setIsLock( bool islock ) { m_layerInfo.setIsLock( islock ); }
	void setIsVisible( bool isVisible ) { m_layerInfo.setIsVisible( isVisible ); }

	virtual void setOrder( int value );

	int getOrder() const { return m_zValue; }
	LayerInfo getLayerInfo() const { return m_layerInfo; }

protected:
	int m_zValue = 0;
	MapScene* m_mapScene;
	LayerInfo m_layerInfo;
};

class TileLayer : public Layer
{
	friend class WorkspaceWidget;
	friend class MapScene;
	friend class LayerAddCommand;
	friend class LayerDeleteCommand;
	friend class Tile;

public:
	TileLayer( MapScene* mapScene, int zValue );
	~TileLayer();

	virtual void setOrder( int value ) override;

	void setTileInfo( int coordX, int coordY, const TileInfo& tileInfo );
	void setTileInfo( int index, const TileInfo& tileInfo );

	TileInfo getTileInfo( int coordX, int coordY );
	TileInfo getTileInfo( int index );

private:
	QList<Tile*> m_tileList;
};

class Tile : public QGraphicsRectItem
{
	friend class WorkspaceWidget;
	friend class MapScene;
	friend class DrawCommand;
	friend class LayerDeleteCommand;

public:
	Tile( MapScene* scene, TileLayer* layer, QGraphicsItem* parent = Q_NULLPTR );

	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;

	void setTileInfo( TileInfo tileInfo ) { m_tileInfo = tileInfo; }
	TileInfo getTileInfo() { return m_tileInfo; }

protected:

private:
	TileInfo m_tileInfo;
	MapScene* m_mapScene;
	TileLayer* m_layer;
};