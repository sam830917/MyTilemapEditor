#pragma once

#include "Core/Tileset.h"
#include "Core/TileInfo.h"
#include <QDockWidget>
#include <QTabWidget>
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
	Q_OBJECT

public:
	TilePalette(Tileset* tileset, QObject *parent = Q_NULLPTR);

private:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
	virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;

public:
	ePaletteSelectMode m_selectMode = ePaletteSelectMode::PALETTE_MULTI_SELECT;
	Tileset* m_tileset;
	int m_currentIndex = 0;
	QSize m_selectedRegionSize = QSize( 1, 1 );
	QList<TileInfo> m_selectedIndexes;
	QGraphicsRectItem* m_selectFrame;
	QPointF m_startPos;
};

class TilesetWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit TilesetWidget( const QString& title, QWidget* parent = Q_NULLPTR );

	void addTilesetIntoProject( Tileset* tileset );
private:
	void tabCurrentChanged(int index);

public slots:
	void addTileset();
	void tilesetRenamed( const QString& path, const QString& oldName, const QString& newName );
	void closeAllTab();

private:
	QList<TilePalette*> m_tilePaletteList;
	QTabWidget* m_tilesetTabWidget;
};