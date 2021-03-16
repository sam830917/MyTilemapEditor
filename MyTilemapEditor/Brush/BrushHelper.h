#pragma once

#include <QObject>
#include <QJSValue>
#include "Core/TileInfo.h"
#include "Brush/BrushCommon.h"

class BrushHelper : public QObject
{
	Q_OBJECT
public:
	Q_INVOKABLE BrushHelper( QObject* parent = nullptr );
	~BrushHelper();

	Q_INVOKABLE void setTile( int coordX, int coordY, TileInfo* tileInfo );
	Q_INVOKABLE void eraseTile( int coordX, int coordY );
	Q_INVOKABLE bool isSameTile( int coordX, int coordY, TileInfo* tileInfo );
	Q_INVOKABLE bool isContainTile( int coordX, int coordY, const QJSValue& value );
	Q_INVOKABLE int getRandomInRange( int low, int high );

	QList<TileModified> popReadyToPaintCoordList();

private:
	QList<TileModified> m_readyToPaintCoordList;
};