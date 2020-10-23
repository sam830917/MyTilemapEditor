#pragma once
#include "Brush/Brush.h"

class AlwaysSameTileBrush : public Brush
{
public:
	AlwaysSameTileBrush();
	~AlwaysSameTileBrush();

	virtual void paint( const QPoint& currentCoord, const MapInfo& mapInfo ) override;
	virtual void erase( const QPoint& currentCoord, const MapInfo& mapInfo ) override;
	virtual QList<AddBrushItem*> createAddDialogItem() override;

private:
	TileInfo m_tile;
};