#include "TileInfo.h"
#include "Tileset.h"

TileInfo::TileInfo( const Tileset* tileset, int index )
	:m_tileset(tileset),
	m_index(index)
{
}

TileInfo::TileInfo( const TileInfo& copyFrom )
{
	if ( copyFrom.isValid() )
	{
		m_index = copyFrom.m_index;
		m_tileset = copyFrom.m_tileset;
	}
}

TileInfo::TileInfo()
{
}

TileInfo::~TileInfo()
{
}

QPixmap TileInfo::getTileImage()
{
	QPoint point = m_tileset->getPoint( m_index );
	return m_tileset->getImage()->copy( point.x(), point.y(), m_tileset->getTileSize().width(), m_tileset->getTileSize().height() );
}

bool TileInfo::operator==( const TileInfo* compare ) const
{
	return *this == *compare;
}

void TileInfo::operator=( const TileInfo& compare )
{
	this->m_tileset = compare.m_tileset;
	this->m_index = compare.m_index;
}

bool TileInfo::operator==( const TileInfo& compare ) const
{
	if ( !m_tileset || !compare.getTileset() )
	{
		return m_tileset == compare.m_tileset;
	}
	return *m_tileset == *compare.m_tileset && m_index == compare.m_index;
}

bool TileInfo::operator!=( const TileInfo& compare ) const
{
	return m_tileset != compare.m_tileset || m_index != compare.m_index;
}
