#include "Layer.h"
#include "Core/MapScene.h"
#include "Core/Tileset.h"

Layer::Layer( MapScene* mapScene, int zValue )
	:m_mapScene(mapScene)
{
	QSize mapSize = m_mapScene->m_mapInfo.getMapSize();
	QSize tileSize = m_mapScene->m_mapInfo.getTileSize();

	m_tileList.reserve( mapSize.height() * mapSize.width() );
	// Create Tiles
	for( int y = 0; y < mapSize.height(); ++y )
	{
		for( int x = 0; x < mapSize.width(); ++x )
		{
			Tile* tile = new Tile( m_mapScene, this );
			tile->setRect( qreal( x * tileSize.width() ), qreal( y * tileSize.height() ), tileSize.width(), tileSize.height() );
			tile->setZValue( -zValue );
			m_mapScene->addItem( tile );
			m_tileList.push_back( tile );
		}
	}
}

Layer::~Layer()
{
	for ( Tile* tile : m_tileList )
	{
		delete tile;
	}
}

void Layer::setOrder( int value )
{
	for ( Tile* tile : m_tileList )
	{
		tile->setZValue( -value );
	}
}

Tile::Tile( MapScene* scene, Layer* layer, QGraphicsItem* parent /*= Q_NULLPTR */ )
	:m_mapScene( scene ),
	m_layer(layer)
{
}

void Tile::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
	if( m_tileInfo.isValid() && m_layer->getLayerInfo().isVisible() )
	{
		QRectF rect = boundingRect();
		QSize& tileSize = m_tileInfo.getTileset()->getTileSize();
		QSize& mapTileSize = m_mapScene->getMapInfo().getTileSize();
		QSize sizeDiff = QSize( mapTileSize.width() - tileSize.width(), mapTileSize.height() - tileSize.height() );
		QPoint point = QPoint( rect.x() + 0.5f, rect.y() + 0.5f + sizeDiff.height() );
		painter->drawPixmap( point.x(), point.y(), tileSize.width(), tileSize.height(), m_tileInfo.getTileImage() );
	}
}