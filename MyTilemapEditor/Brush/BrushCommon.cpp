#include "Brush/BrushCommon.h"
#include "Brush/Brush.h"
#include "Core/TileInfo.h"
#include "Core/TileSelector.h"
#include <QLineEdit>

#define CAST_WIDGET_ITEM(type) \
	type* from = dynamic_cast<type*>(fromItem->m_widgetItem); \
	type* to = dynamic_cast<type*>(toItem->m_widgetItem);

template <typename T>
void createBrushUIItem( const QString& name, T* val, QList<AddBrushItem*>& itemList ) {};

void createBrushUIItem( const QString& name, QString* val, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* stringItem = new AddBrushItem();
	QLineEdit* stringInput = new QLineEdit();
	stringInput->setText( *val );
	QObject::connect( stringInput, &QLineEdit::textChanged, [=]( const QString& newValue ) { *val = newValue; } );
	stringItem->m_name = name;
	stringItem->m_widgetItem = stringInput;
	stringItem->m_type = eItemType::STRING;
	itemList.push_back( stringItem );
}

void createBrushUIItem( const QString& name, TileInfo* val, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* tileItem = new AddBrushItem();
	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	tileSelector->setTileInfo( *val );
	QObject::connect( tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo ) { *val = tileInfo; } );
	tileItem->m_name = name;
	tileItem->m_widgetItem = tileSelector;
	tileItem->m_type = eItemType::TILE_INFO;
	itemList.push_back( tileItem );
}

void assignBrushItem( AddBrushItem* fromItem, AddBrushItem* toItem )
{
	if ( fromItem->m_type != toItem->m_type )
		return;
	
	switch( fromItem->m_type )
	{
	case eItemType::STRING:
	{
		CAST_WIDGET_ITEM( QLineEdit );
		to->setText( from->text() );
		break;
	}
	case eItemType::TILE_INFO:
	{
		CAST_WIDGET_ITEM( TileSelector );
		to->setTileInfo( from->getTileinfo() );
		break;
	}
	default:
		return;
	}
}

Brush* copyBrush( Brush* referBrush, BrushType* type )
{
	Brush* newBrush = type->m_constructorFunction();

	QList<AddBrushItem*> oldBrushItems = referBrush->createAddDialogItem();
	QList<AddBrushItem*> newBrushItems = newBrush->createAddDialogItem();

	for ( int i = 0; i < oldBrushItems.size(); ++i )
	{
		AddBrushItem* oldItem = oldBrushItems[i];
		AddBrushItem* newItem = newBrushItems[i];

		assignBrushItem( oldItem, newItem );
	}
	return newBrush;
}