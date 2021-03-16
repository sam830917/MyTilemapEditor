#pragma once

#include "ui_TileSelectorUI.h"
#include "Core/TileInfo.h"
#include "Core/TilePalette.hpp"
#include <QDialog>
#include <QBoxLayout>

QT_FORWARD_DECLARE_CLASS( QTabWidget )
class TilePalette;

class SelectTileDialog : public QDialog
{
	Q_OBJECT

public:
	SelectTileDialog( QWidget* parent = Q_NULLPTR );
	~SelectTileDialog();

	TileInfo getSelectSingleTile() const;
	QList<TileInfo> getSelectTiles() const;
	QSize getSelectTileRegion() const;

	void setSelectTileMode( ePaletteSelectMode selectMode );

public:
	Ui::TileSelectorUI m_ui;
	QTabWidget* m_tabWidget;
	QList<TilePalette*> m_tilePaletteList;
};