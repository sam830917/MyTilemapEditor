#pragma once

#include "Core/TileInfo.h"
#include <QGraphicsView>
#include <QList>

class TileGridSelectorScene;
class TileGridItem;

enum class eTileGridType
{
	EDGE,
	CORNER,
	EDGE_AND_CORNER
};

class TileGridSelector : public QGraphicsView
{
public:
	TileGridSelector( eTileGridType type = eTileGridType::EDGE_AND_CORNER, const QSize& size = QSize( 50, 50 ) );
	~TileGridSelector();

	QList<bool> getGridState() const;
	void setGridState( QList<bool> states ) const;
	TileInfo getTileinfo() const;
	void setTileInfo( TileInfo tileinfo );

private:
	QSize m_size;
	TileGridSelectorScene* m_scene = Q_NULLPTR;
	eTileGridType m_type = eTileGridType::EDGE_AND_CORNER;
};