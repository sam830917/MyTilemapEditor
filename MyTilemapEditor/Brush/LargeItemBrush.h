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
	QList<TileInfo> m_tileList;
	int m_width = 0;
	int m_height = 0;
};