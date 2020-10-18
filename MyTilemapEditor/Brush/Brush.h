#pragma once

#include <QPoint>
#include <QMap>
#include "Core/TileInfo.h"

struct TileModified
{
	TileModified( QPoint coordinate, TileInfo tileInfo ) :m_coordinate(coordinate), m_tileInfo(tileInfo) {}

	QPoint m_coordinate;
	TileInfo m_tileInfo;
};

class Brush
{
public:
	Brush();
	~Brush();

	virtual void paint( const QPoint& currentCoord );
	virtual void erase( const QPoint& currentCoord );

	QString getName() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	QList<TileModified> popReadyToPaintCoordList();

protected:
	void setTile( QPoint coordinate, TileInfo tileInfo );
	void eraseTile( QPoint coordinate );

private:
	QString m_name = "Unnamed";

	QList<TileModified> m_readyToPaintCoordList;
};