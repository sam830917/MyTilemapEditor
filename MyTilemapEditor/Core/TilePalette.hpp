#pragma once

#include "Core/Tileset.h"
#include "Core/TileInfo.h"
#include <QGraphicsScene>
#include <QPoint>
#include <QSize>

enum class ePaletteSelectMode
{
	PALETTE_SINGLE_SELECT,
	PALETTE_MULTI_SELECT,
};

class TilePalette : public QGraphicsScene
{
public:
	TilePalette( Tileset* tileset, QObject* parent = Q_NULLPTR );

	void updateGlobalTilePalette();

private:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;

public:
	ePaletteSelectMode m_selectMode = ePaletteSelectMode::PALETTE_MULTI_SELECT;
	Tileset* m_tileset;
	int m_currentIndex = 0;
	int m_updateGlobalTilePalette = true;
	QSize m_selectedRegionSize = QSize( 1, 1 );
	QList<TileInfo> m_selectedTileInfos;
	QGraphicsRectItem* m_selectFrame;
	QPointF m_startPos;
};