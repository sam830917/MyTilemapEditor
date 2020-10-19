#include "Brush/BrushCommon.h"
#include "Core/TileInfo.h"
#include "Core/TileSelector.h"
#include <QLineEdit>

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
	itemList.push_back( stringItem );
}

void createBrushUIItem( const QString& name, TileInfo* val, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* tileItem = new AddBrushItem();
	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	QObject::connect( tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo ) { *val = tileInfo; } );
	tileItem->m_name = name;
	tileItem->m_widgetItem = tileSelector;
	itemList.push_back( tileItem );
}
