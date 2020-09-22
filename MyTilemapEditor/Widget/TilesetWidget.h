#pragma once

#include "../Core/Tileset.h"
#include "../Core/TileInfo.h"
#include <QDockWidget>
#include <QTabWidget>
#include <QGraphicsScene>

class TilePalette : public QGraphicsScene
{
	Q_OBJECT

public:
	TilePalette(QObject *parent = Q_NULLPTR);

private:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

public:
	Tileset* m_tileset;
	int m_currentIndex = 0;
	QGraphicsRectItem* m_selectFrame;
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

private:
	QList<TilePalette*> m_tilePaletteList;
	QTabWidget* m_tilesetTabWidget;

};