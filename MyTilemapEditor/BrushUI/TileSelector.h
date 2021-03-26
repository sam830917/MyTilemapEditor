#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include "Core/TileInfo.h"
#include "Core/TilePalette.hpp"

class TileSelectorScene;
class TileItem;

class TileSelector : public QGraphicsView
{
	Q_OBJECT
	friend class TileSelectorScene;

public:
	TileSelector( const QSize& size = QSize( 50, 50 ) );
	~TileSelector();

	QSize getSize() const { return m_size; }
	TileInfo getTileinfo() const;
	void setTileInfo( TileInfo tileinfo );
	void setIsMutiSelect( bool isMultiSelect );

	TileItem* getTileItem() const;

signals:
	void tileChanged( TileInfo tileInfo );

private:
	TileSelectorScene* m_scene;
	QSize m_size;
	bool m_isMultiSelect = false;
};

class TileItem : public QObject, public QGraphicsRectItem
{
	Q_OBJECT
public:
	TileItem( const QSize& size, ePaletteSelectMode selectMode, const QPointF& position = QPointF(0.f, 0.f) );
	~TileItem();

	TileInfo getTileInfo() const { return m_firstSelectedTile; }
	void setTileInfo( TileInfo tileInfo );

	void setSelectMode( ePaletteSelectMode mode ) { m_selectMode = mode; }

signals:
	void selectedExtraTiles( QList<TileInfo>& extraTiles );

protected:
	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	ePaletteSelectMode m_selectMode = ePaletteSelectMode::PALETTE_SINGLE_SELECT;
	TileInfo m_firstSelectedTile;
	QSize m_size;
	QPointF m_position;
};