#include "Brush/Brush.h"
#include "Utils/ProjectCommon.h"

Brush::Brush()
{
}

Brush::~Brush()
{
}

void Brush::paint( const QPoint& currentCoord )
{
	setTile( currentCoord, getCurrentTile() );
}

void Brush::erase( const QPoint& currentCoord )
{
	eraseTile( currentCoord );
}

void Brush::setTile( QPoint coordinate, TileInfo tileInfo )
{
	m_readyToPaintCoordList.push_back( TileModified( coordinate, tileInfo ) );
}

void Brush::eraseTile( QPoint coordinate )
{
	m_readyToPaintCoordList.push_back( TileModified( coordinate, TileInfo() ) );
}

QList<TileModified> Brush::popReadyToPaintCoordList()
{
	QList<TileModified> copy = m_readyToPaintCoordList;
	m_readyToPaintCoordList.clear();
	return copy;
}
