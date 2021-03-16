#include "Brush/BrushHelper.h"
#include "Core/MapScene.h"
#include "Utils/ProjectCommon.h"
#include <QDebug>
#include <QTime>
#include <QPoint>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QRandomGenerator>

extern MapScene* g_currentMapScene;
extern QJSEngine* g_currentJsEngine;

BrushHelper::BrushHelper( QObject* parent /*= nullptr */ )
	:QObject( parent )
{
}

BrushHelper::~BrushHelper()
{

}

void BrushHelper::setTile( int coordX, int coordY, TileInfo* tileInfo )
{
	if ( !tileInfo )
	{
		debugPrintError( "setTile error : Tile is null!" );
	}
	TileInfo tile(*tileInfo);
	TileModified tileModified( QPoint( coordX, coordY ), tile );
	m_readyToPaintCoordList.push_back( tileModified );
}

void BrushHelper::eraseTile( int coordX, int coordY )
{
	TileModified tileModified( QPoint( coordX, coordY ), TileInfo() );
	m_readyToPaintCoordList.push_back( tileModified );
}

bool BrushHelper::isSameTile( int coordX, int coordY, TileInfo* tileInfo )
{
	if( !g_currentMapScene || !tileInfo )
	{
		debugPrintError( "setTile error : Tile is null!" );
		return false;
	}
	int layerIndex = g_currentMapScene->getCurrentLayerIndex();
	if( layerIndex == -1 )
	{
		return false;
	}
	int tileIndex = g_currentMapScene->m_mapInfo.getIndex( QPoint( coordX, coordY ) );
	TileInfo tile(g_currentMapScene->m_layers[layerIndex]->m_tileList[tileIndex]->getTileInfo());
	return tile == *tileInfo;
}

bool BrushHelper::isContainTile( int coordX, int coordY, const QJSValue& value )
{
	if( !g_currentMapScene )
	{
		return false;
	}
	int layerIndex = g_currentMapScene->getCurrentLayerIndex();
	if( layerIndex == -1 )
	{
		return false;
	}
	if ( g_currentMapScene->m_mapInfo.isOutOfBound( QPoint( coordX, coordY ) ) )
	{
		return false;
	}
	int tileIndex = g_currentMapScene->m_mapInfo.getIndex( QPoint( coordX, coordY ) );
	TileInfo tile( g_currentMapScene->m_layers[layerIndex]->m_tileList[tileIndex]->getTileInfo() );
	if ( value.isArray() )
	{
		int length = value.property( "length" ).toInt();
		QJSValueIterator it( value );
		while( it.hasNext() )
		{
			it.next();

			TileInfo* value = qobject_cast<TileInfo*>(it.value().toQObject());
			if ( tile.isValid() && value && tile == *value )
			{
				return true;
			}
		}
	}
	return false;
}

int BrushHelper::getRandomInRange( int low, int high )
{
	return QRandomGenerator::global()->bounded(low, high);
}

QList<TileModified> BrushHelper::popReadyToPaintCoordList()
{
	QList<TileModified> copy = m_readyToPaintCoordList;
	m_readyToPaintCoordList.clear();
	return copy;
}
