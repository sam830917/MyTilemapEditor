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

	AddBrushItem* tileItem = new AddBrushItem();
	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	QObject::connect( tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo ) { this->m_tile = tileInfo; } );

	tileItem->m_name = "Tile";
	tileItem->m_widgetItem = tileSelector;
	items.push_back( tileItem );
	return items;
}
