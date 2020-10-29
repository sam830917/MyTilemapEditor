#include "Brush/BrushParser.h"
#include "Brush/BrushHelper.h"
#include "Core/TileInfo.h"
#include "Core/Tileset.h"
#include "Core/TileSelector.h"
#include "Core/TileInfoListContainer.h"
#include "Utils/XmlUtils.h"
#include <QJSEngine>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>
#include <QJSValueIterator>
#include <QDebug>
#include <QLineEdit>

BrushParser::BrushParser()
{
	// Load all brush files
	initialBrushFile( QCoreApplication::applicationDirPath() + "/Brushes/Brush.js" );
}

void BrushParser::initialBrushFile( const QString& filePath )
{
	QFileInfo jsFileInfo( filePath );
	QJSEngine* jsEngine = createJSEngine( filePath );

	// Run Startup
	QJSValue startupFun = jsEngine->globalObject().property( "Startup" );
	QJSValue result = startupFun.call();
	if( result.isError() )
	{
		qDebug() << "Error!";
		qDebug() << result.property( "name" ).toString() << ", " \
			<< result.property( "message" ).toString();
		qDebug() << result.property( "lineNumber" ).toInt();
	}

	QJSValue exposeVariables = jsEngine->globalObject().property( "exposeVariables" );
	if( exposeVariables.isArray() )
	{
		BrushFile brushFile;
		brushFile.m_filePath = filePath;
		QJSValueIterator it( exposeVariables );
		QList<BrushItemInfo> defaultBrush;
		while( it.hasNext() )
		{
			it.next();

			QJSValue object = it.value();
			BrushItemInfo item;
			QString type = object.property( "type" ).toString();
			QString labelName = object.property( "labelName" ).toString();
			if( "String" == type )
			{
				item.m_itemType = eItemType::STRING;
			}
			else if( "Tile" == type )
			{
				item.m_itemType = eItemType::TILE_INFO;
			}
			else if( "TileArray" == type )
			{
				item.m_itemType = eItemType::TILE_INFO_LIST;
			}
			QString id = object.property( "id" ).toString();
			item.m_id = id;
			item.m_labelName = labelName;
			if( eItemType::UNKNOWN != item.m_itemType )
			{
				defaultBrush.push_back( item );
			}
		}
		brushFile.m_itemList = defaultBrush;
		m_brushItemMap[jsFileInfo.completeBaseName()] = brushFile;
	}
	delete jsEngine;
}

BrushParser::~BrushParser()
{
}

QList<AddBrushItem*> BrushParser::createBrushUI( const QString& brushName )
{
	QList<AddBrushItem*> itemList;
	QList<BrushItemInfo> brushItemList = m_brushItemMap[brushName].m_itemList;

	for ( BrushItemInfo brushItem : brushItemList )
	{
		if ( eItemType::STRING == brushItem.m_itemType )
		{
			createStringUI( brushItem.m_labelName, itemList );
		}
		else if( eItemType::TILE_INFO == brushItem.m_itemType )
		{
			createTileUI( brushItem.m_labelName, itemList );
		}
		else if( eItemType::TILE_INFO_LIST == brushItem.m_itemType )
		{
			QList<TileInfo> tileList;
			createTileListUI( brushItem.m_labelName, itemList, tileList );
		}
	}

	return itemList;
}

QList<AddBrushItem*> BrushParser::createBrushUIByCurrentBrush( int index )
{
	QList<AddBrushItem*> itemList;
	QString brushName = getBrushFileName(index);
	QList<BrushItemInfo> brushItemList = m_brushItemMap[brushName].m_itemList;
	QJSEngine* jsEngine = m_brushes[index];

	for( int i = 0; i < brushItemList.size(); ++i )
	{
		BrushItemInfo brushItem = brushItemList[i];

		if( eItemType::STRING == brushItem.m_itemType )
		{
			QJSValue value = jsEngine->globalObject().property( brushItem.m_id );
			if ( value.isUndefined() )
			{
				createStringUI( brushItem.m_labelName, itemList );
			}
			else
			{
				QString str = value.toString();
				createStringUI( brushItem.m_labelName, itemList, str );
			}
		}
		else if( eItemType::TILE_INFO == brushItem.m_itemType )
		{
			TileInfo* value = qobject_cast<TileInfo*>(jsEngine->globalObject().property( brushItem.m_id ).toQObject());
			createTileUI( brushItem.m_labelName, itemList, TileInfo( *value ) );
		}
		else if( eItemType::TILE_INFO_LIST == brushItem.m_itemType )
		{
			QJSValue valueArray = jsEngine->globalObject().property( brushItem.m_id );
			if ( valueArray.isArray() )
			{
				const int length = valueArray.property("length").toInt();
				QList<TileInfo> tileList;
				for (int i = 0; i < length; ++i)
				{
					TileInfo* value = qobject_cast<TileInfo*>(valueArray.property( i ).toQObject());
					tileList.push_back( TileInfo( *value ) );
				}
				createTileListUI( brushItem.m_labelName, itemList, tileList );
			}
		}
	}

	return itemList;
}

bool BrushParser::loadBrushFile( const QString& filePath )
{
	XmlDocument* doc = new XmlDocument;
	doc->LoadFile( filePath.toStdString().c_str() );
	if( doc->Error() )
	{
		return false;
	}

	XmlElement* root = doc->RootElement();
	if( root )
	{
		QString brushName = parseXmlAttribute( *root, "brushName", QString() );
		if( brushName.isEmpty() )
			return false;

		BrushFile brushFile = m_brushItemMap[brushName];
		QJSEngine* jsEngine = createJSEngine( brushFile.m_filePath );
		jsEngine->globalObject().setProperty( "BrushFilePath", brushFile.m_filePath );
		XmlElement* brushItem = root->FirstChildElement( "BrushItem" );
		int index = 0;
		while( brushItem )
		{
			BrushItemInfo itemInfo = brushFile.m_itemList[index];
			if( index >= brushFile.m_itemList.size() )
				break;

			QString type = parseXmlAttribute( *brushItem, "type", QString() );
			if( "STRING" == type )
			{
				QString value = parseXmlAttribute( *brushItem, "value", QString() );
				if( !value.isEmpty() )
				{
					jsEngine->globalObject().setProperty( itemInfo.m_id, value );
				}
			}
			else if ( "TILE_INFO" == type )
			{
				QString tilesetFilePath = parseXmlAttribute( *brushItem, "tileset", QString() );
				int tileIndex = parseXmlAttribute( *brushItem, "index", 0 );
				if( !tilesetFilePath.isEmpty() )
				{
					tilesetFilePath = getProjectRootPath() + "/" + tilesetFilePath;
					TileInfo* tileinfo = new TileInfo( convertToTileset( tilesetFilePath ), tileIndex );
					if ( tileinfo )
					{
						QJSValue objectvalue = jsEngine->newQObject( tileinfo );
						jsEngine->globalObject().setProperty( itemInfo.m_id, objectvalue );
					}
				}
				else
				{
					TileInfo* tileinfo = new TileInfo();
					QJSValue objectvalue = jsEngine->newQObject( tileinfo );
					jsEngine->globalObject().setProperty( itemInfo.m_id, objectvalue );
				}
			}
			else if( "TILE_INFO_LIST" == type )
			{
				QJSValue objectvalueArray = jsEngine->newArray();
				int i = 0;
				for( XmlElement* tileEle = brushItem->FirstChildElement( "TileList" ); tileEle; tileEle = tileEle->NextSiblingElement( "TileList" ) )
				{
					QString tilesetFilePath = parseXmlAttribute( *tileEle, "tileset", QString() );
					int tileIndex = parseXmlAttribute( *tileEle, "index", 0 );

					if( !tilesetFilePath.isEmpty() )
					{
						tilesetFilePath = getProjectRootPath() + "/" + tilesetFilePath;
						TileInfo* tileinfo = new TileInfo( convertToTileset( tilesetFilePath ), tileIndex );
						QJSValue objectvalue = jsEngine->newQObject( tileinfo );
						objectvalueArray.setProperty( i, objectvalue );
					}
					else
					{
						TileInfo* tileinfo = new TileInfo();
						QJSValue objectvalue = jsEngine->newQObject( tileinfo );
						objectvalueArray.setProperty( i, objectvalue );
					}
					i++;
				}
				jsEngine->globalObject().setProperty( itemInfo.m_id, objectvalueArray );
			}

			index++;
			brushItem = brushItem->NextSiblingElement( "BrushItem" );
		}

		jsEngine->globalObject().setProperty( "FilePath", filePath );
		m_brushes.push_back(jsEngine);
	}
	return true;
}

bool BrushParser::saveBrushAsFile( QList<AddBrushItem*> items, const QString& saveFilePath, const QString& brushFilePath, QJSEngine* jsEngine )
{
	QFileInfo jsFileInfo( brushFilePath );
	QList<BrushItemInfo> brushItemList = m_brushItemMap[jsFileInfo.completeBaseName()].m_itemList;
	if ( !jsEngine )
	{
		jsEngine = createJSEngine( brushFilePath );
	}

	XmlDocument* xmlDocument = new XmlDocument();
	XmlElement* root = xmlDocument->NewElement( "Brush" );
	QFileInfo brushFileInfo( brushFilePath );
	root->SetAttribute( "brushName", brushFileInfo.completeBaseName().toStdString().c_str() );
	xmlDocument->LinkEndChild( root );
	for ( int i = 0; i < items.size(); ++i )
	{
		AddBrushItem* item = items[i];
		BrushItemInfo brushItem = brushItemList[i];
		XmlElement* itemEle = xmlDocument->NewElement( "BrushItem" );

		switch( item->m_type )
		{
		case eItemType::STRING:
		{
			QLineEdit* input = dynamic_cast<QLineEdit*>(item->m_widgetItem);
			itemEle->SetAttribute( "type", "STRING" );
			itemEle->SetAttribute( "value", input->text().toStdString().c_str() );
			jsEngine->globalObject().setProperty( brushItem.m_id, input->text() );
			break;
		}
		case eItemType::TILE_INFO:
		{
			TileSelector* selector = dynamic_cast<TileSelector*>(item->m_widgetItem);
			itemEle->SetAttribute( "type", "TILE_INFO" );
			TileInfo* tileinfo = new TileInfo(selector->getTileinfo());
			if( tileinfo->isValid() )
			{
				itemEle->SetAttribute( "tileset", tileinfo->getTileset()->getRelativeFilePath().toStdString().c_str() );
				itemEle->SetAttribute( "index", tileinfo->getIndex() );
			}
			else
			{
				itemEle->SetAttribute( "tileset", "" );
				itemEle->SetAttribute( "index", -1 );
			}
			QJSValue objectvalue = jsEngine->newQObject( tileinfo );
			jsEngine->globalObject().setProperty( brushItem.m_id, objectvalue );
			break;
		}
		case eItemType::TILE_INFO_LIST:
		{
			TileInfoListContainer* tileListContainer = dynamic_cast<TileInfoListContainer*>(item->m_treeItem);
			itemEle->SetAttribute( "type", "TILE_INFO_LIST" );

			QJSValue objectvalueArray = jsEngine->newArray( tileListContainer->getTileSelectorList().size() );
			for( int i = 0; i < tileListContainer->getTileSelectorList().size(); ++i )
			{
				TileSelector* tileSelector = tileListContainer->getTileSelectorList()[i];
				XmlElement* childEle = xmlDocument->NewElement( "TileList" );
				TileInfo* tileinfo = new TileInfo( tileSelector->getTileinfo() );
				if ( tileinfo->isValid() )
				{
					childEle->SetAttribute( "tileset", tileinfo->getTileset()->getRelativeFilePath().toStdString().c_str() );
				}
				else
				{
					childEle->SetAttribute( "tileset", "" );
				}
				childEle->SetAttribute( "index", tileinfo->getIndex() );
				itemEle->LinkEndChild( childEle );

				QJSValue objectvalue = jsEngine->newQObject( tileinfo );
				objectvalueArray.setProperty( i, objectvalue );
			}

			const int length = objectvalueArray.property( "length" ).toInt();
			jsEngine->globalObject().setProperty( brushItem.m_id, objectvalueArray );
			break;
		}
		default:
			break;
		}
		root->LinkEndChild( itemEle );
	}
	jsEngine->globalObject().setProperty( "FilePath", saveFilePath );
	jsEngine->globalObject().setProperty( "BrushFilePath", brushFilePath );
	m_brushes.push_back( jsEngine );
 	saveXmlFile( *xmlDocument, saveFilePath );

	return true;
}

bool BrushParser::modifyBrushAsFile( QList<AddBrushItem*> items, const QString& saveFilePath, int brushIndex )
{
	QJSEngine* jsEngine = m_brushes[brushIndex];
	QString brushFile =  jsEngine->globalObject().property( "BrushFilePath" ).toString();
	QString oldPathFile =  jsEngine->globalObject().property( "FilePath" ).toString();

	bool isSuccess = saveBrushAsFile( items, saveFilePath, brushFile, jsEngine );
	if ( !isSuccess )
	{
		return false;
	}

	if ( saveFilePath != oldPathFile )
	{
		QFile file( oldPathFile );
		file.remove();
	}
	return true;
}

QList<TileModified> BrushParser::getPaintMapResult( int brushIndex, const QPoint& coord, eDrawTool tool )
{
	QList<TileModified> emptyList;
	if ( tool != eDrawTool::BRUSH && tool != eDrawTool::ERASER )
	{
		return emptyList;
	}
	QJSEngine* jsEngine = m_brushes[brushIndex];

	QJSValue toolFunction;
	if ( tool == eDrawTool::BRUSH )
	{
		toolFunction = jsEngine->globalObject().property( "Draw" );
	}
	else if ( tool == eDrawTool::ERASER )
	{
		toolFunction = jsEngine->globalObject().property( "Erase" );
	}
	else
	{
		return emptyList;
	}
	QJSValue object = QJSValue( coord.x() );
	QJSValue object2 = QJSValue( coord.y() );
	QJSValueList list;
	list.push_back( object );
	list.push_back( object2 );
	QJSValue result = toolFunction.call( list );
	if( result.isError() )
	{
		qDebug() << "Error!";
		qDebug() << result.property( "name" ).toString() << ", " << result.property( "message" ).toString();
		qDebug() << result.property( "lineNumber" ).toInt();
		return emptyList;
	}

	BrushHelper* helper = qobject_cast<BrushHelper*>(jsEngine->evaluate( "helper" ).toQObject());
	if( helper )
	{
		return helper->popReadyToPaintCoordList();
	}
	return emptyList;
}

QString BrushParser::getFilePathByIndex( int index )
{
 	QJSEngine* jsEngine = m_brushes[index];
	return jsEngine->globalObject().property( "FilePath" ).toString();
}

QString BrushParser::getFileName( int index )
{
	QString filePath = getFilePathByIndex( index );
	QFileInfo fileinfo( filePath );
	return fileinfo.completeBaseName();
}

QString BrushParser::getBrushFilePathByIndex( int index )
{
	QJSEngine* jsEngine = m_brushes[index];
	return jsEngine->globalObject().property( "BrushFilePath" ).toString();
}

QString BrushParser::getBrushFileName( int index )
{
	QString filePath = getBrushFilePathByIndex(index);
	QFileInfo fileinfo(filePath);
	return fileinfo.completeBaseName();
}

void BrushParser::createStringUI( const QString& labelName, QList<AddBrushItem*>& itemList, const QString& value )
{
	AddBrushItem* stringItem = new AddBrushItem();
	QLineEdit* stringInput = new QLineEdit();
	stringInput->setText( value );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, labelName );
	stringItem->m_name = labelName;
	stringItem->m_widgetItem = stringInput;
	stringItem->m_type = eItemType::STRING;
	stringItem->m_treeItem = item;
	itemList.push_back( stringItem );
}

void BrushParser::createTileUI( const QString& labelName, QList<AddBrushItem*>& itemList, const TileInfo& value )
{
	AddBrushItem* tileItem = new AddBrushItem();
	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	tileSelector->setTileInfo( value );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, labelName );
	tileItem->m_name = labelName;
	tileItem->m_widgetItem = tileSelector;
	tileItem->m_type = eItemType::TILE_INFO;
	tileItem->m_treeItem = item;
	itemList.push_back( tileItem );
}

void BrushParser::createTileListUI( const QString& labelName, QList<AddBrushItem*>& itemList, QList<TileInfo> tileList )
{
	AddBrushItem* tileListItem = new AddBrushItem();
	TileInfoListContainer* t = new TileInfoListContainer();
	for ( TileInfo tile : tileList )
	{
		t->appendToInitialList( tile );
	}
	t->setText( 0, labelName );
	tileListItem->m_name = labelName;
	tileListItem->m_widgetItem = t->getTopItemWidget();
	tileListItem->m_type = eItemType::TILE_INFO_LIST;
	tileListItem->m_treeItem = t;
	itemList.push_back( tileListItem );
}

QJSEngine* BrushParser::createJSEngine( const QString& filePath )
{
	QFile jsFile( filePath );
	QJSEngine* jsEngine = new QJSEngine();

	QTextStream in( &jsFile );
	if( jsFile.open( QFile::ReadOnly | QFile::Text ) )
	{
		QString jsStr = in.readAll();
		QJSValue tileinfoMetaObject = jsEngine->newQMetaObject( &TileInfo::staticMetaObject );
		jsEngine->globalObject().setProperty( "TileInfo", tileinfoMetaObject );
		QJSValue metaObject = jsEngine->newQMetaObject( &BrushHelper::staticMetaObject );
		jsEngine->globalObject().setProperty( "BrushHelper", metaObject );

		BrushHelper* brushHelper = new BrushHelper;
		QJSValue objectvalue = jsEngine->newQObject( brushHelper );
		jsEngine->globalObject().setProperty( "helper", objectvalue );

		QJSValue errorValue = jsEngine->evaluate( jsStr );
		if( errorValue.isError() )
		{
			qDebug() << "Error!";
			qDebug() << errorValue.property( "name" ).toString() << ", " \
				<< errorValue.property( "message" ).toString();
			qDebug() << errorValue.property( "lineNumber" ).toInt();
			return nullptr;
		}
		return jsEngine;
	}
	return nullptr;
}
