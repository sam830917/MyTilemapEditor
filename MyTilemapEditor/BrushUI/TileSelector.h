#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include "Core/TileInfo.h"

class TileSelectorScene;

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

signals:
	void tileChanged( TileInfo tileInfo );

private:
	TileSelectorScene* m_scene;
	QSize m_size;
};

class TileItem : public QGraphicsRectItem
{

public:
	TileItem( const QSize& size, const QPointF& position = QPointF(0.f, 0.f) );
	~TileItem();

	TileInfo getTileInfo() const { return m_selectedTile; }
	void setTileInfo( TileInfo tileInfo );

protected:
	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	TileInfo m_selectedTile;
	QSize m_size;
	QPointF m_position;
};