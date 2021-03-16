#pragma once

#include "Core/Tileset.h"
#include "Core/TileInfo.h"
#include "Core/TilePalette.hpp"
#include <QDockWidget>
#include <QTabWidget>
#include <QGraphicsScene>
#include <QPoint>
#include <QSize>

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