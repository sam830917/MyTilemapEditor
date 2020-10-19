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

QBoxLayout* LargeItemBrush::createAddDialogUI()
{
	QBoxLayout* layout = CREATE_BASIC_LAYOUT;

	QBoxLayout* tileLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	QLabel* nameLabel = new QLabel;
	nameLabel->setText( "Tile" );
	tileLayout->addWidget( nameLabel, 0, Qt::AlignTop );
	tileLayout->addWidget( tileSelector, 0, Qt::AlignTop );
	QObject::connect( tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo ) { this->m_tile = tileInfo; } );
	layout->addLayout( tileLayout );

	return layout;
}
