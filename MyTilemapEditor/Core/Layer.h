#pragma once

#include "Core/TileInfo.h"
#include "Core/LayerInfo.h"
#include <QGraphicsRectItem>
#include <QList>

class MapScene;
class Tile;
class MarkerTile;

class Layer
{
public:
	Layer( MapScene* mapScene, int zValue );
	virtual ~Layer();

	void setLayerInfo( const LayerInfo& layerInfo ) { m_layerInfo = layerInfo; }
	void setName( const QString& name ) { m_layerInfo.setName( name ); }
	void setIsLock( bool islock ) { m_layerInfo.setIsLock( islock ); }
	void setIsVisible( bool isVisible ) { m_layerInfo.setIsVisible( isVisible ); }
	void setColor( const QColor& color ) { m_layerInfo.setColor( color ); }

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
	friend class DrawCommand;
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

class MarkerLayer : public Layer
{
	friend class WorkspaceWidget;
	friend class MapScene;
	friend class LayerAddCommand;
	friend class LayerDeleteCommand;
	friend class LayerColorChangeCommand;

public:
	MarkerLayer( MapScene* mapScene, int zValue );
	~MarkerLayer();

	virtual void setOrder( int value ) override;

	QColor getColor() const { return m_layerInfo.getColor(); }
	bool IsMarked( int coordX, int coordY );
	bool IsMarked( int index );

	void markTile( int coordX, int coordY, bool isMark );
	void markTile( int index, bool isMark );

private:
	QList<MarkerTile*> m_tileList;
};

class Tile : public QGraphicsRectItem
{
	friend class WorkspaceWidget;
	friend class MapScene;
	friend class DrawCommand;
	friend class LayerDeleteCommand;

public:
	Tile( MapScene* scene, TileLayer* layer, QGraphicsItem* parent = Q_NULLPTR );

	void setTileInfo( TileInfo tileInfo ) { m_tileInfo = tileInfo; }
	TileInfo getTileInfo() { return m_tileInfo; }

protected:
	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;

private:
	TileInfo m_tileInfo;
	MapScene* m_mapScene;
	TileLayer* m_layer;
};

class MarkerTile : public QGraphicsRectItem
{
	friend class MarkerLayer;

public:
	MarkerTile( MapScene* scene, MarkerLayer* layer, QGraphicsItem* parent = Q_NULLPTR );

	bool isMarked() const { return m_marked; }

protected:
	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;

private:
	bool m_marked = false;
	MapScene* m_mapScene;
	MarkerLayer* m_layer;
};