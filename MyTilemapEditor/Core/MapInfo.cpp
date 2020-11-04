#include "MapInfo.h"

MapInfo::MapInfo()
{

}

MapInfo::MapInfo( int mapWidth, int mapHeight, int tileWidth, int tileHeight )
	:m_mapHeight(mapHeight),
	m_mapWidth(mapWidth),
	m_tileHeight(tileHeight),
	m_tileWidth(tileWidth)
{

}

MapInfo::MapInfo( const QSize& mapSize, const QSize& tileSize )
	:m_mapHeight( mapSize.height() ),
	m_mapWidth( mapSize.width() ),
	m_tileHeight( tileSize.height() ),
	m_tileWidth( tileSize.width() )
{

}

MapInfo::~MapInfo()
{

}

bool MapInfo::isOutOfBound( const QPoint& coord )
{
	return coord.x() >= m_mapWidth || coord.y() >= m_mapHeight || coord.x() < 0 || coord.y() < 0 ;
}
