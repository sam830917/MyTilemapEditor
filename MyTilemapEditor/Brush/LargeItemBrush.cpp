#include "Brush/LargeItemBrush.h"
#include "Core/TileSelector.h"
#include <QGraphicsView>
#include <QLineEdit>
#include <QLabel>

BRUSH_TYPE( "Large Item Brush", LargeItemBrush );

LargeItemBrush::LargeItemBrush()
	:Brush()
{
	m_name = "Large Item Brush";
}

LargeItemBrush::~LargeItemBrush()
{
}

void LargeItemBrush::paint( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	int listSize = m_tileList.size();
	for( int y = 0; y < m_height; ++y )
	{
		for( int x = 0; x < m_width; ++x )
		{
			int index = y * m_width + x;
			if( index < m_tileList.size() )
			{
				setTile( QPoint( currentCoord.x() + x, currentCoord.y() + y ), m_tileList[index] );
			}
		}
	}
}

void LargeItemBrush::erase( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	int listSize = m_tileList.size();
	for( int y = 0; y < m_height; ++y )
	{
		for( int x = 0; x < m_width; ++x )
		{
			eraseTile( QPoint( currentCoord.x() + x, currentCoord.y() + y ) );
		}
	}
}

QList<AddBrushItem*> LargeItemBrush::createAddDialogItem()
{
	QList<AddBrushItem*> items = CREATE_BASIC_ITEM;
	CREATE_ITEM( "Tiles", m_tileList, items );
	CREATE_ITEM( "Width", m_width, items );
	CREATE_ITEM( "Height", m_height, items );

	return items;
}
