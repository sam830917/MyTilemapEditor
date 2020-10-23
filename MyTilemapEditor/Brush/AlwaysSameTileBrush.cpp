#include "Brush/AlwaysSameTileBrush.h"

BRUSH_TYPE( "Always Same Tile Brush", AlwaysSameTileBrush );

AlwaysSameTileBrush::AlwaysSameTileBrush()
	:Brush()
{
	m_name = "Always Same Tile Brush";
}

AlwaysSameTileBrush::~AlwaysSameTileBrush()
{
}

void AlwaysSameTileBrush::paint( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	setTile( currentCoord, m_tile );
}

void AlwaysSameTileBrush::erase( const QPoint& currentCoord, const MapInfo& mapInfo )
{
	eraseTile( currentCoord );
}

QList<AddBrushItem*> AlwaysSameTileBrush::createAddDialogItem()
{
	QList<AddBrushItem*> items = CREATE_BASIC_ITEM;
	CREATE_ITEM( "Tile", m_tile, items );

	return items;
}
