#include "Brush/BrushHelper.h"
#include <QDebug>
#include <QTime>
#include <QPoint>

BrushHelper::BrushHelper( QObject* parent /*= nullptr */ )
	:QObject( parent )
{

}

BrushHelper::~BrushHelper()
{

}

void BrushHelper::SetTile( int coordX, int coordY, TileInfo* tileInfo )
{
	TileInfo tile(*tileInfo);
	TileModified tileModified( QPoint( coordX, coordY ), tile );
	m_readyToPaintCoordList.push_back( tileModified );
}

void BrushHelper::EraseTile( int coordX, int coordY )
{
	TileModified tileModified( QPoint( coordX, coordY ), TileInfo() );
	m_readyToPaintCoordList.push_back( tileModified );
}

int BrushHelper::GetRandomInRange( int low, int high )
{
	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());
	int range = ( ( high ) - low ) + low;
	if ( range == 0 )
	{
		return 0;
	}
	return qrand() % range;
}

QList<TileModified> BrushHelper::popReadyToPaintCoordList()
{
	QList<TileModified> copy = m_readyToPaintCoordList;
	m_readyToPaintCoordList.clear();
	return copy;
}
