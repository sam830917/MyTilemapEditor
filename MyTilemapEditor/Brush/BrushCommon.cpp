#include "Brush/BrushCommon.h"
#include "Brush/Brush.h"
#include "Core/TileInfo.h"
#include "Core/Tileset.h"
#include "Core/TileSelector.h"
#include "Core/TileInfoListContainer.h"
#include "Core//IntInput.h"
#include "Utils/XmlUtils.h"
#include "Utils/ProjectCommon.h"
#include <QLineEdit>
#include <QPushButton>

#define CAST_WIDGET_ITEM(type, item) dynamic_cast<type*>(item->m_widgetItem);

template <typename T>
void createBrushUIItem( const QString& name, T* val, QList<AddBrushItem*>& itemList ) {};

void createBrushUIItem( const QString& name, QString* val, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* stringItem = new AddBrushItem();
	QLineEdit* stringInput = new QLineEdit();
	stringInput->setText( *val );
	QObject::connect( stringInput, &QLineEdit::textChanged, [=]( const QString& newValue ) { *val = newValue; } );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, name );
	stringItem->m_name = name;
	stringItem->m_widgetItem = stringInput;
	stringItem->m_type = eItemType::STRING;
	stringItem->m_treeItem = item;
	itemList.push_back( stringItem );
}

void createBrushUIItem( const QString& name, TileInfo* val, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* tileItem = new AddBrushItem();
	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	tileSelector->setTileInfo( *val );
	QObject::connect( tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo ) { *val = tileInfo; } );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, name );
	tileItem->m_name = name;
	tileItem->m_widgetItem = tileSelector;
	tileItem->m_type = eItemType::TILE_INFO;
	tileItem->m_treeItem = item;
	itemList.push_back( tileItem );
}

void createBrushUIItem( const QString& name, QList<TileInfo>* val, QList<AddBrushItem*>& itemList )
{
// 	AddBrushItem* tileListItem = new AddBrushItem();
// 	TileInfoListContainer* t = new TileInfoListContainer( val );
// 	t->setText( 0, name );
// 	tileListItem->m_name = name;
// 	tileListItem->m_widgetItem = t->getTopItemWidget();
// 	tileListItem->m_type = eItemType::TILE_INFO_LIST;
// 	tileListItem->m_treeItem = t;
// 	itemList.push_back( tileListItem );
}

void createBrushUIItem( const QString& name, int* val, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* stringItem = new AddBrushItem();
	IntInput* intInput = new IntInput(val);
	QObject::connect( intInput, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]( int newValue ) {  *val = newValue; } );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, name );
	stringItem->m_name = name;
	stringItem->m_widgetItem = intInput;
	stringItem->m_type = eItemType::INT;
	stringItem->m_treeItem = item;
	itemList.push_back( stringItem );
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
	case eItemType::INT:
	{
		IntInput* from = CAST_WIDGET_ITEM( IntInput, fromItem );
		IntInput* to = CAST_WIDGET_ITEM( IntInput, toItem );
		to->setValue( from->value() );
		break;
	}
	case eItemType::TILE_INFO:
	{
		TileSelector* from = CAST_WIDGET_ITEM( TileSelector, fromItem );
		TileSelector* to = CAST_WIDGET_ITEM( TileSelector, toItem );
		to->setTileInfo( from->getTileinfo() );
		break;
	}
	case eItemType::TILE_INFO_LIST:
	{
		TileInfoListContainer* from = dynamic_cast<TileInfoListContainer*>(fromItem->m_treeItem);
		TileInfoListContainer* to = dynamic_cast<TileInfoListContainer*>(toItem->m_treeItem);
		int emptyTileNumber = 0;
		for ( int i = 0; i < from->getTileSelectorList().size(); ++i )
		{
			TileSelector* fromTS = from->getTileSelectorList()[i];
			if ( fromTS->getTileinfo().isValid() )
			{
				to->addTileSelectorList();
				TileSelector* toTS = to->getTileSelectorList()[i - emptyTileNumber];
				toTS->setTileInfo( fromTS->getTileinfo() );
			}
			else
			{
				emptyTileNumber++;
			}
		}
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

bool setBrushItemXmlElement( XmlElement& itemEle, AddBrushItem* brushItem, XmlDocument& xmlDocument )
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
	case eItemType::INT:
	{
		IntInput* item = CAST_WIDGET_ITEM( IntInput, brushItem );
		itemEle.SetAttribute( "type", "INT" );
		itemEle.SetAttribute( "value", item->value() );
		break;
	}
	case eItemType::TILE_INFO:
	{
		TileSelector* item = CAST_WIDGET_ITEM( TileSelector, brushItem );
		itemEle.SetAttribute( "type", "TILE_INFO" );
		TileInfo tileinfo = item->getTileinfo();
		if ( tileinfo.isValid() )
		{
			itemEle.SetAttribute( "tileset", tileinfo.getTileset()->getRelativeFilePath().toStdString().c_str() );
			itemEle.SetAttribute( "index", tileinfo.getIndex() );
		}
		break;
	}
	case eItemType::TILE_INFO_LIST:
	{
		TileInfoListContainer* item = dynamic_cast<TileInfoListContainer*>(brushItem->m_treeItem);
		itemEle.SetAttribute( "type", "TILE_INFO_LIST" );
		for ( TileSelector* tileSelector : item->getTileSelectorList() )
		{
			XmlElement* childEle = xmlDocument.NewElement( "TileList" );
			TileInfo tileinfo = tileSelector->getTileinfo();
			if( tileinfo.isValid() )
			{
				childEle->SetAttribute( "tileset", tileinfo.getTileset()->getRelativeFilePath().toStdString().c_str() );
				childEle->SetAttribute( "index", tileinfo.getIndex() );
				itemEle.LinkEndChild( childEle );
			}
		}
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
		if ( !setBrushItemXmlElement( *itemEle, item, *xmlDocument ) )
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
		else if( "INT" == type )
		{
			int value = parseXmlAttribute( *itemEle, "value", 0 );
			AddBrushItem* brushItem = brushItems[index];
			if( brushItem->m_type == eItemType::INT )
			{
				IntInput* item = CAST_WIDGET_ITEM( IntInput, brushItem );
				item->setValue( value );
			}
		}
		else if( "TILE_INFO" == type )
		{
			QString tilesetFilePath = parseXmlAttribute( *itemEle, "tileset", QString() );
			int tileIndex = parseXmlAttribute( *itemEle, "index", 0 );
			if ( !tilesetFilePath.isEmpty() )
			{
				AddBrushItem* brushItem = brushItems[index];
				if( brushItem->m_type == eItemType::TILE_INFO )
				{
					TileSelector* item = CAST_WIDGET_ITEM( TileSelector, brushItem );
					QString filePath = getProjectRootPath() + "/" + tilesetFilePath;
					TileInfo tileinfo( convertToTileset( filePath ), tileIndex );
					item->setTileInfo( tileinfo );
				}
			}
		}
		else if( "TILE_INFO_LIST" == type )
		{
			for ( XmlElement* tileEle = itemEle->FirstChildElement( "TileList" ); tileEle; tileEle = tileEle->NextSiblingElement( "TileList" ) )
			{
				QString tilesetFilePath = parseXmlAttribute( *tileEle, "tileset", QString() );
				int tileIndex = parseXmlAttribute( *tileEle, "index", 0 );

				if( !tilesetFilePath.isEmpty() )
				{
					AddBrushItem* brushItem = brushItems[index];
					if( brushItem->m_type == eItemType::TILE_INFO_LIST )
					{
						QString filePath = getProjectRootPath() + "/" + tilesetFilePath;
						TileInfo tileinfo( convertToTileset( filePath ), tileIndex );
						TileInfoListContainer* item = dynamic_cast<TileInfoListContainer*>(brushItem->m_treeItem);
						item->addTileSelectorList();
						item->getTileSelectorList()[item->getTileSelectorList().size() - 1]->setTileInfo(tileinfo);
					}
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

bool isListType( eItemType type )
{
	return type == eItemType::TILE_INFO_LIST;
}
