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
public:
	Brush();
	~Brush();

	virtual void paint( const QPoint& currentCoord, const MapInfo& mapInfo );
	virtual void erase( const QPoint& currentCoord, const MapInfo& mapInfo );
	virtual QList<AddBrushItem*> createAddDialogItem();

	QString getName() const { return m_name; }
	BrushType* getBrushType() const { return m_brushType; }
	void setName( const QString& name ) { m_name = name; }
	void setBrushType( BrushType* brushType ) { m_brushType = brushType; }
	QList<TileModified> popReadyToPaintCoordList();

	static QList<BrushType*> getAllBrushType();

protected:
	void setTile( QPoint coordinate, TileInfo tileInfo );
	void eraseTile( QPoint coordinate );

protected:
	QString m_name = "Unnamed";
	QList<TileModified> m_readyToPaintCoordList;

private:
	BrushType* m_brushType;

};