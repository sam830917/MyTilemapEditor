#include "Layer.h"
#include "Core/MapScene.h"
#include "Core/Tileset.h"

Layer::Layer( MapScene* mapScene, int zValue )
	:m_mapScene(mapScene),
	m_zValue(zValue)
{
}

Layer::~Layer()
{
}

void Layer::setOrder( int value )
{
	m_zValue = value;
}

TileLayer::TileLayer( MapScene* mapScene, int zValue )
	:Layer( mapScene, zValue )
{
	m_layerInfo.setLayerType(eLayerType::TILE_LAYER);
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

TileLayer::~TileLayer()
{
	for ( Tile* tile : m_tileList )
	{
		delete tile;
	}
}

void TileLayer::setOrder( int value )
{
	for ( Tile* tile : m_tileList )
	{
		tile->setZValue( -value );
	}
	Layer::setOrder(value);
}

void TileLayer::setTileInfo( int coordX, int coordY, const TileInfo& tileInfo )
{
	int tileIndex = m_mapScene->m_mapInfo.getIndex( QPoint( coordX, coordY ) );
	m_tileList[tileIndex]->setTileInfo( tileInfo );
	m_tileList[tileIndex]->update();
}

void TileLayer::setTileInfo( int index, const TileInfo& tileInfo )
{
	m_tileList[index]->setTileInfo( tileInfo );
	m_tileList[index]->update();
}

TileInfo TileLayer::getTileInfo( int coordX, int coordY )
{
	int tileIndex = m_mapScene->m_mapInfo.getIndex( QPoint( coordX, coordY ) );
	return m_tileList[tileIndex]->getTileInfo();
}

TileInfo TileLayer::getTileInfo( int index )
{
	return m_tileList[index]->getTileInfo();
}

Tile::Tile( MapScene* scene, TileLayer* layer, QGraphicsItem* parent /*= Q_NULLPTR */ )
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

MarkerLayer::MarkerLayer( MapScene* mapScene, int zValue )
	:Layer( mapScene, zValue )
{
	m_layerInfo.setLayerType( eLayerType::MARKER_LAYER );
	QSize mapSize = m_mapScene->m_mapInfo.getMapSize();
	QSize tileSize = m_mapScene->m_mapInfo.getTileSize();

	m_tileList.reserve( mapSize.height() * mapSize.width() );
	// Create Tiles
	for( int y = 0; y < mapSize.height(); ++y )
	{
		for( int x = 0; x < mapSize.width(); ++x )
		{
			MarkerTile* tile = new MarkerTile( m_mapScene, this );
			tile->setRect( qreal( x * tileSize.width() ), qreal( y * tileSize.height() ), tileSize.width(), tileSize.height() );
			tile->setZValue( -zValue );
			m_mapScene->addItem( tile );
			m_tileList.push_back( tile );
		}
	}
	m_layerInfo.setColor( QColorConstants::Yellow );
}

MarkerLayer::~MarkerLayer()
{
	for( MarkerTile* tile : m_tileList )
	{
		delete tile;
	}
}

void MarkerLayer::setOrder( int value )
{
	for( MarkerTile* tile : m_tileList )
	{
		tile->setZValue( -value );
	}
	Layer::setOrder( value );
}

bool MarkerLayer::IsMarked( int coordX, int coordY )
{
	int tileIndex = m_mapScene->m_mapInfo.getIndex( QPoint( coordX, coordY ) );
	return m_tileList[tileIndex]->m_marked;
}

void MarkerLayer::markTile( int coordX, int coordY, bool isMark )
{
	int tileIndex = m_mapScene->m_mapInfo.getIndex( QPoint( coordX, coordY ) );
	m_tileList[tileIndex]->m_marked = isMark;
	m_tileList[tileIndex]->update();
}

void MarkerLayer::markTile( int index, bool isMark )
{
	m_tileList[index]->m_marked = isMark;
	m_tileList[index]->update();
}

MarkerTile::MarkerTile( MapScene* scene, MarkerLayer* layer, QGraphicsItem* parent /*= Q_NULLPTR */ )
	:m_mapScene( scene ),
	m_layer( layer )
{
}

void MarkerTile::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= Q_NULLPTR */ )
{
	if ( m_marked && m_layer->getLayerInfo().isVisible() )
	{
		QRectF rect = boundingRect();
		QSize& mapTileSize = m_mapScene->getMapInfo().getTileSize();
		QPoint point = QPoint( rect.x() + 0.5f, rect.y() + 0.5f );
		QColor color = m_layer->getColor();
		painter->setBrush( QBrush( QColor( color.red(), color.green(), color.blue(), 50 ) ) );
		painter->setPen( QPen( QColor( color.red(), color.green(), color.blue(), 50 ) ) );
		painter->drawRect( point.x(), point.y(), mapTileSize.width(), mapTileSize.height() );
	}
}
