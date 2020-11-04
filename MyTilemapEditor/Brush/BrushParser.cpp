#include "Brush/BrushParser.h"
#include "Brush/BrushHelper.h"
#include "Brush/Jsconsole.h"
#include "Core/TileInfo.h"
#include "Core/Tileset.h"
#include "Core/TileSelector.h"
#include "Core/TileInfoListContainer.h"
#include "Core/TileGridListContainer.h"
#include "Core/TileGridSelector.h"
#include "Core//IntInput.h"
#include "Utils/XmlUtils.h"
#include <QJSEngine>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>
#include <QJSValueIterator>
#include <QLineEdit>
#include <QDir>

QString g_brushLibStr;
QJSEngine* g_currentJsEngine = nullptr;

BrushParser::BrushParser()
{
	// Load all brush files
	QDir brushDirectory( QCoreApplication::applicationDirPath() + "/Brushes" );
	QStringList brushFiles = brushDirectory.entryList( QStringList() << "*.js" << "*.JS", QDir::Files );
	for( QString fileName : brushFiles )
	{
		QString filePath = brushDirectory.filePath( fileName );
		initialBrushFile( filePath );
	}
}

void BrushParser::initialBrushFile( const QString& filePath )
{
	QFileInfo jsFileInfo( filePath );
	QJSEngine* jsEngine = createJSEngine( filePath );

	if ( !jsEngine )
		return;

	// Run Startup
	QJSValue startupFun = jsEngine->globalObject().property( "Startup" );
	QJSValue result = startupFun.call();
	if( result.isError() )
	{
		debugPrint( "Call Startup function failed in file \"" + filePath + "\"" );
		debugPrint( result.property( "name" ).toString() + ", " + result.property( "message" ).toString() +
			" in line " + result.property( "lineNumber" ).toString() );
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
			else if( "Int" == type )
			{
				item.m_itemType = eItemType::INT;
			}
			else if( "TileGridBool" == type )
			{
				item.m_itemType = eItemType::TILE_GRID_BOOL;
			}
			else if( "TileGridBoolListEdge" == type )
			{
				item.m_itemType = eItemType::TILE_GRID_BOOL_LIST_EDGE;
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
		else if( eItemType::INT == brushItem.m_itemType )
		{
			createIntUI( brushItem.m_labelName, itemList );
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
		else if( eItemType::TILE_GRID_BOOL == brushItem.m_itemType )
		{
			createTileGridUI( brushItem.m_labelName, itemList );
		}
		else if( eItemType::TILE_GRID_BOOL_LIST_EDGE == brushItem.m_itemType )
		{
			QList<TileInfo> tileList;
			QList<QList<bool>> statesList;
			createTileGridListUI( brushItem.m_labelName, itemList, eTileGridType::EDGE, tileList, statesList );
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
		else if( eItemType::INT == brushItem.m_itemType )
		{
			QJSValue value = jsEngine->globalObject().property( brushItem.m_id );
			if( value.isUndefined() )
			{
				createIntUI( brushItem.m_labelName, itemList );
			}
			else
			{
				int val = value.toInt();
				createIntUI( brushItem.m_labelName, itemList, val );
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
		else if( eItemType::TILE_GRID_BOOL_LIST_EDGE == brushItem.m_itemType )
		{
			QJSValue valueArray = jsEngine->globalObject().property( brushItem.m_id );
			if ( valueArray.isArray() )
			{
				const int length = valueArray.property("length").toInt();
				QList<TileInfo> tileList;
				QList<QList<bool>> statesList;
				for (int i = 0; i < length; ++i)
				{
					QList<bool> states;
					QJSValue val = valueArray.property( i );
					TileInfo* value = qobject_cast<TileInfo*>(val.property( "tile" ).toQObject());
					states.push_back(val.property( "top" ).toBool());
					states.push_back(val.property( "left" ).toBool());
					states.push_back(val.property( "right" ).toBool());
					states.push_back(val.property( "bottom" ).toBool());
					tileList.push_back( TileInfo( *value ) );
					statesList.push_back( states );
				}
				createTileGridListUI( brushItem.m_labelName, itemList, eTileGridType::EDGE, tileList, statesList );
			}
		}
	}

	return itemList;
}

bool BrushParser::loadBrushFile( const QString& filePath )
{
	// Check is already exist
	for ( int i = 0; i < m_brushes.size(); ++i )
	{
		QString path = getFilePathByIndex(i);
		if ( filePath == path )
		{
			return false;
		}
	}

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
		if ( !jsEngine )
			return false;

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
			else if( "INT" == type )
			{
				int value = parseXmlAttribute( *brushItem, "value", 0 );
				jsEngine->globalObject().setProperty( itemInfo.m_id, value );
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
			else if( "TILE_GRID_BOOL_LIST_EDGE" == type )
			{
				QJSValue objectvalueArray = jsEngine->newArray();
				int i = 0;
				for( XmlElement* tileEle = brushItem->FirstChildElement( "TileGridList" ); tileEle; tileEle = tileEle->NextSiblingElement( "TileGridList" ) )
				{
					QString tilesetFilePath = parseXmlAttribute( *tileEle, "tileset", QString() );
					int tileIndex = parseXmlAttribute( *tileEle, "index", 0 );
					QJSValue val = jsEngine->newObject();

					if( !tilesetFilePath.isEmpty() )
					{
						tilesetFilePath = getProjectRootPath() + "/" + tilesetFilePath;
						TileInfo* tileinfo = new TileInfo( convertToTileset( tilesetFilePath ), tileIndex );
						QJSValue objectvalue = jsEngine->newQObject( tileinfo );
						val.setProperty( "tile", objectvalue );
					}
					else
					{
						TileInfo* tileinfo = new TileInfo();
						QJSValue objectvalue = jsEngine->newQObject( tileinfo );
						val.setProperty( "tile", objectvalue );
					}
					QString gridStateStr = parseXmlAttribute( *tileEle, "gridState", QString() );
					QStringList state = gridStateStr.split(QLatin1Char(','));
					if ( state.size() < 4 )
					{
						continue;
					}
					val.setProperty( "top", state[0] == "1" ? true : false );
					val.setProperty( "left", state[1] == "1" ? true : false );
					val.setProperty( "right", state[2] == "1" ? true : false );
					val.setProperty( "bottom", state[3] == "1" ? true : false );
					objectvalueArray.setProperty( i, val );
					i++;
				}
				jsEngine->globalObject().setProperty( itemInfo.m_id, objectvalueArray );
			}

			index++;
			brushItem = brushItem->NextSiblingElement( "BrushItem" );
		}

		jsEngine->globalObject().setProperty( "FilePath", filePath );
		m_brushes.push_back( jsEngine );

		// call update function
		QJSValue updateFun = jsEngine->globalObject().property( "Update" );
		QJSValue result = updateFun.call();
		if( result.isError() )
		{
			debugPrint( "Call Update function failed in file \"" + filePath + "\"" );
			debugPrint( result.property( "name" ).toString() + ", " + result.property( "message" ).toString() +
				" in line " + result.property( "lineNumber" ).toString() );
		}
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
		case eItemType::INT:
		{
			IntInput* input = dynamic_cast<IntInput*>(item->m_widgetItem);
			itemEle->SetAttribute( "type", "INT" );
			itemEle->SetAttribute( "value", input->value() );
			jsEngine->globalObject().setProperty( brushItem.m_id, input->value() );
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
			jsEngine->globalObject().setProperty( brushItem.m_id, objectvalueArray );
			break;
		}
		case eItemType::TILE_GRID_BOOL_LIST_EDGE:
		{
			TileGridListContainer* tileGridListContainer = dynamic_cast<TileGridListContainer*>(item->m_treeItem);
			itemEle->SetAttribute( "type", "TILE_GRID_BOOL_LIST_EDGE" );
			QJSValue objectvalueArray = jsEngine->newArray( tileGridListContainer->getTileSelectorList().size() );
			for( int i = 0; i < tileGridListContainer->getTileSelectorList().size(); ++i )
			{
				TileGridSelector* grid = tileGridListContainer->getTileGridSelectorList()[i];
				TileSelector* tileSelector = tileGridListContainer->getTileSelectorList()[i];
				XmlElement* childEle = xmlDocument->NewElement( "TileGridList" );
				TileInfo* tileinfo = new TileInfo( tileSelector->getTileinfo() );
				if( tileinfo->isValid() )
				{
					childEle->SetAttribute( "tileset", tileinfo->getTileset()->getRelativeFilePath().toStdString().c_str() );
				}
				else
				{
					childEle->SetAttribute( "tileset", "" );
				}
				childEle->SetAttribute( "index", tileinfo->getIndex() );

				QList<bool> states = grid->getGridState();
				QString gridState = QString( "%1,%2,%3,%4" ).arg(states[0]).arg(states[1]).arg(states[2]).arg(states[3]);
				childEle->SetAttribute( "gridState", gridState.toStdString().c_str() );
				itemEle->LinkEndChild( childEle );

				QJSValue objectvalue = jsEngine->newObject();
				objectvalue.setProperty( "tile", jsEngine->newQObject( tileinfo ) );
				objectvalue.setProperty( "top", states[0] );
				objectvalue.setProperty( "left", states[1] );
				objectvalue.setProperty( "right", states[2] );
				objectvalue.setProperty( "bottom", states[3] );

				objectvalueArray.setProperty( i, objectvalue );
			}
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

	// call update function
	QJSValue updateFun = jsEngine->globalObject().property( "Update" );
	QJSValue result = updateFun.call();
	if( result.isError() )
	{
		debugPrint( "Call Update function failed in file \"" + saveFilePath + "\"" );
		debugPrint( result.property( "name" ).toString() + ", " + result.property( "message" ).toString() +
			" in line " + result.property( "lineNumber" ).toString() );
	}

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

void BrushParser::deleteBrush( int index )
{
	delete m_brushes[index];
	m_brushes.removeAt(index);
}

void BrushParser::deleteAllBrush()
{
	for ( int i = 0; i < m_brushes.size(); ++i )
	{
		delete m_brushes[i];
	}
	m_brushes.clear();
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
	QString functionName = "painting function";
	if ( tool == eDrawTool::BRUSH )
	{
		functionName = "Draw";
		toolFunction = jsEngine->globalObject().property( "Draw" );
	}
	else if ( tool == eDrawTool::ERASER )
	{
		functionName = "Erase";
		toolFunction = jsEngine->globalObject().property( "Erase" );
	}
	else
	{
		return emptyList;
	}
	g_currentJsEngine = jsEngine;
	QJSValue object = QJSValue( coord.x() );
	QJSValue object2 = QJSValue( coord.y() );
	QJSValueList list;
	list.push_back( object );
	list.push_back( object2 );
	QJSValue result = toolFunction.call( list );
	if( result.isError() )
	{
		debugPrint( "Call " + functionName + " function failed!" );
		debugPrint( result.property( "name" ).toString() + ", " + result.property( "message" ).toString() + 
			" in line " + result.property( "lineNumber" ).toString() );
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

void BrushParser::createTileGridUI( const QString& labelName, QList<AddBrushItem*>& itemList )
{
	AddBrushItem* tileItem = new AddBrushItem();
	TileGridSelector* tileSelector = new TileGridSelector( eTileGridType::EDGE );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, labelName );
	tileItem->m_name = labelName;
	tileItem->m_widgetItem = tileSelector;
	tileItem->m_type = eItemType::TILE_INFO;
	tileItem->m_treeItem = item;
	itemList.push_back( tileItem );
}

void BrushParser::createTileGridListUI( const QString& labelName, QList<AddBrushItem*>& itemList, eTileGridType type, QList<TileInfo> tileList, QList<QList<bool>> statesList )
{
	AddBrushItem* tileListItem = new AddBrushItem();
	TileGridListContainer* t = new TileGridListContainer();
	for( TileInfo tile : tileList )
	{
		t->appendToInitialList( tile );
	}
	for( QList<bool> states : statesList )
	{
		t->appendToInitialList( states );
	}
	t->setText( 0, labelName );
	tileListItem->m_name = labelName;
	tileListItem->m_widgetItem = t->getTopItemWidget();
	switch( type )
	{
	case eTileGridType::EDGE:
		t->setGridType( type );
		tileListItem->m_type = eItemType::TILE_GRID_BOOL_LIST_EDGE;
		break;
	case eTileGridType::CORNER:
		break;
	case eTileGridType::EDGE_AND_CORNER:
		break;
	default:
		break;
	}
	tileListItem->m_treeItem = t;
	itemList.push_back( tileListItem );
}

void BrushParser::createIntUI( const QString& labelName, QList<AddBrushItem*>& itemList, int value )
{
	AddBrushItem* stringItem = new AddBrushItem();
	IntInput* intInput = new IntInput( value );
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText( 0, labelName );
	stringItem->m_name = labelName;
	stringItem->m_widgetItem = intInput;
	stringItem->m_type = eItemType::INT;
	stringItem->m_treeItem = item;
	itemList.push_back( stringItem );
}

QJSEngine* BrushParser::createJSEngine( const QString& filePath )
{
	QFile jsFile( filePath );
	QJSEngine* jsEngine = new QJSEngine();

	if ( g_brushLibStr.isEmpty() )
	{
		QFile file( QCoreApplication::applicationDirPath() + "/brush-lib.js" );
		QTextStream fileIn( &file );
		if( file.open( QFile::ReadOnly | QFile::Text ) )
		{
			g_brushLibStr = fileIn.readAll();
		}
	}
	jsEngine->evaluate( g_brushLibStr );

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

		QJSValue consoleObj =  jsEngine->newQObject( new JSConsole );
		jsEngine->globalObject().setProperty( "console", consoleObj );

		QJSValue errorValue = jsEngine->evaluate( jsStr );
		if( errorValue.isError() )
		{
			debugPrint( "Creating js engine failed!" );
			debugPrint( errorValue.property( "name" ).toString() + ", " + errorValue.property( "message" ).toString() +
				" in line " + errorValue.property( "lineNumber" ).toString() );
			delete jsEngine;
			return nullptr;
		}
		return jsEngine;
	}
	delete jsEngine;
	return nullptr;
}
