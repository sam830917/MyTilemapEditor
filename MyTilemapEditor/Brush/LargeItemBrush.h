#pragma once

#include "Brush/Brush.h"

class LargeItemBrush : public Brush
{
public:
	LargeItemBrush();
	~LargeItemBrush();

	virtual void paint( const QPoint& currentCoord, const MapInfo& mapInfo ) override;
	virtual void erase( const QPoint& currentCoord, const MapInfo& mapInfo ) override;
	virtual QList<AddBrushItem*> createAddDialogItem() override;

private:
	TileInfo m_tile;
	QList<TileInfo> m_tileList;
};