#pragma once

#include "Core/TileInfo.h"
#include <QGraphicsView>
#include <QList>

class TileGridSelectorScene;
class TileGrid;

enum class eGridState
{
	FREE,
	ALLOW,
	BLOCK
};
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

	QList<eGridState> getGridState() const;

private:
	QSize m_size;
	TileGridSelectorScene* m_scene;
	eTileGridType m_type = eTileGridType::EDGE_AND_CORNER;
};