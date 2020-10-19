#pragma once

#include <QGraphicsView>
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

signals:
	void tileChanged( TileInfo tileInfo );

private:
	TileSelectorScene* m_scene;
	TileInfo m_selectedTile;
	QSize m_size;
};