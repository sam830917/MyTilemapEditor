#include "Brush/BrushCommon.h"
#include "Brush/Brush.h"
#include "Core/TileInfo.h"
#include "Core/Tileset.h"
#include "Core/TileSelector.h"
#include "Utils/XmlUtils.h"
#include "Utils/ProjectCommon.h"
#include <QLineEdit>

#define CAST_WIDGET_ITEM(type, item) dynamic_cast<type*>(item->m_widgetItem);

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
		QLineEdit* from = CAST_WIDGET_ITEM( QLineEdit, fromItem );
		QLineEdit* to = CAST_WIDGET_ITEM( QLineEdit, toItem );
		to->setText( from->text() );
		break;
	}
	case eItemType::TILE_INFO:
	{
		TileSelector* from = CAST_WIDGET_ITEM( TileSelector, fromItem );
		TileSelector* to = CAST_WIDGET_ITEM( TileSelector, toItem );
		to->setTileInfo( from->getTileinfo() );
		break;
	}
	default:
		return;
	}
}

Brush* copyBrush( Brush* referBrush )
{
	Brush* newBrush = referBrush->getBrushType()->m_constructorFunction();

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

bool setBrushItemXmlElement( XmlElement& itemEle, AddBrushItem* brushItem )
{
	switch( brushItem->m_type )
	{
	case eItemType::STRING:
	{
		QLineEdit* item = CAST_WIDGET_ITEM( QLineEdit, brushItem );
		itemEle.SetAttribute( "type", "STRING" );
		itemEle.SetAttribute( "value", item->text().toStdString().c_str() );
		break;
	}
	case eItemType::TILE_INFO:
	{
		TileSelector* item = CAST_WIDGET_ITEM( TileSelector, brushItem );
		itemEle.SetAttribute( "type", "TILE_INFO" );
		TileInfo tileinfo = item->getTileinfo();
		itemEle.SetAttribute( "tileset", tileinfo.getTileset()->getRelativeFilePath().toStdString().c_str() );
		itemEle.SetAttribute( "index", tileinfo.getIndex() );
		break;
	}
	default:
		return false;
	}
	return true;
}

bool saveBrushAsFile( Brush* brush, QString filePath )
{
	// Save as XML
	XmlDocument* xmlDocument = new XmlDocument();
	XmlElement* root = xmlDocument->NewElement( "Brush" );
	root->SetAttribute( "type", brush->getBrushType()->m_displayName.toStdString().c_str() );
	xmlDocument->LinkEndChild( root );

	QList<AddBrushItem*> brushItems = brush->createAddDialogItem();
	for ( AddBrushItem* item : brushItems )
	{
		XmlElement* itemEle = xmlDocument->NewElement( "BrushItem" );
		if ( !setBrushItemXmlElement( *itemEle, item ) )
		{
			return false;
		}
		root->LinkEndChild( itemEle );
	}
	saveXmlFile( *xmlDocument, filePath );

	return true;
}

void loadBrushItemXmlElement( XmlElement* itemEle, Brush* brush )
{
	if ( !itemEle )
		return;
	
	QList<AddBrushItem*> brushItems = brush->createAddDialogItem();

	int index = 0;
	while ( itemEle )
	{
		if ( index >= brushItems.size() )
			return;
		
		QString type = parseXmlAttribute( *itemEle, "type", QString() );
		if ( type.isEmpty() )
		{
			index++;
			itemEle = itemEle->NextSiblingElement( "BrushItem" );
			continue;
		}

		if( "STRING" == type )
		{
			QString value = parseXmlAttribute( *itemEle, "value", QString() );
			if ( !value.isEmpty() )
			{
				AddBrushItem* brushItem = brushItems[index];
				if ( brushItem->m_type == eItemType::STRING )
				{
					QLineEdit* item = CAST_WIDGET_ITEM( QLineEdit, brushItem );
					item->setText( value );
				}
			}
		}
		else if( "TILE_INFO" == type )
		{
			QString tilesetFilePath = parseXmlAttribute( *itemEle, "tileset", QString() );
			int index = parseXmlAttribute( *itemEle, "index", 0 );
			if ( !tilesetFilePath.isEmpty() )
			{
				AddBrushItem* brushItem = brushItems[index];
				if( brushItem->m_type == eItemType::TILE_INFO )
				{
					TileSelector* item = CAST_WIDGET_ITEM( TileSelector, brushItem );
					QString filePath = getProjectRootPath() + "/" + tilesetFilePath;
					TileInfo tileinfo( convertToTileset( filePath ), index );
					item->setTileInfo( tileinfo );
				}
			}
		}

		index++;
		itemEle = itemEle->NextSiblingElement( "BrushItem" );
	}
}

Brush* loadBrush( const QString& brushFilePath )
{
	QFile file( brushFilePath );
	if( !file.exists() )
	{
		return nullptr;
	}

	XmlDocument* doc = new XmlDocument;
	doc->LoadFile( brushFilePath.toStdString().c_str() );
	if( doc->Error() )
	{
		return nullptr;
	}

	XmlElement* root = doc->RootElement();
	if ( root )
	{
		QString type = parseXmlAttribute( *root, "type", QString() );
		if ( type.isEmpty() )
		{
			return nullptr;
		}
		QList<BrushType*> brushTypeList = Brush::getAllBrushType();
		BrushType* brushType = nullptr;
		for ( BrushType* t : brushTypeList )
		{
			if ( t->m_displayName == type )
			{
				brushType = t;
				break;
			}
		}
		if ( !brushType )
		{
			return nullptr;
		}
		Brush* brush = brushType->m_constructorFunction();
		brush->setBrushType( brushType );
		XmlElement* brushItem = root->FirstChildElement( "BrushItem" );
		loadBrushItemXmlElement( brushItem, brush );

		return brush;
	}
	return nullptr;
}
