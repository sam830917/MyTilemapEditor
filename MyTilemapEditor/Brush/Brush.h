#pragma once

#include <QPoint>
#include <QList>
#include "Core/TileInfo.h"
#include "Core/MapInfo.h"
#include "Brush/BrushCommon.h"

struct TileModified
{
	TileModified( QPoint coordinate, TileInfo tileInfo ) :m_coordinate(coordinate), m_tileInfo(tileInfo) {}

	QPoint m_coordinate;
	TileInfo m_tileInfo;
};

class Brush
{
	friend class BrushWidget;

public:
	Brush();
	~Brush();

	virtual void paint( const QPoint& currentCoord, const MapInfo& mapInfo );
	virtual void erase( const QPoint& currentCoord, const MapInfo& mapInfo );
	virtual QList<AddBrushItem*> createAddDialogItem();

	QString getName() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	QList<TileModified> popReadyToPaintCoordList();

private:
	static QList<BrushType*> getAllBrushType();

protected:
	void setTile( QPoint coordinate, TileInfo tileInfo );
	void eraseTile( QPoint coordinate );

public:
	QString m_name = "Unnamed";

protected:
	QList<TileModified> m_readyToPaintCoordList;

};