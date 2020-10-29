#pragma once

#include <QObject>
#include "Core/TileInfo.h"
#include "Brush/BrushCommon.h"

class BrushHelper : public QObject
{
	Q_OBJECT
public:
	Q_INVOKABLE BrushHelper( QObject* parent = nullptr );
	~BrushHelper();

	Q_INVOKABLE void SetTile( int coordX, int coordY, TileInfo* tileInfo );
	Q_INVOKABLE void EraseTile( int coordX, int coordY );

	QList<TileModified> popReadyToPaintCoordList();

private:
	QList<TileModified> m_readyToPaintCoordList;

};