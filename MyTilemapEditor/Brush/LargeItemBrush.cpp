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
	setTile( currentCoord, m_tile );
}

void LargeItemBrush::erase( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	Brush::erase( currentCoord, mapInfo );
}

QList<AddBrushItem*> LargeItemBrush::createAddDialogItem()
{
	QList<AddBrushItem*> items = CREATE_BASIC_ITEM;
	CREATE_ITEM( "Tile", m_tile, items );
	CREATE_ITEM( "Tiles", m_tileList, items );
	CREATE_ITEM( "Width", m_width, items );
	CREATE_ITEM( "Height", m_height, items );

	return items;
}
