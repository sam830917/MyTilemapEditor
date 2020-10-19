#pragma once

#include <QDialog>
#include <QBoxLayout>
#include "ui_TileSelectorUI.h"
#include "Core/TileInfo.h"

QT_FORWARD_DECLARE_CLASS( QTabWidget )
class TilePalette;

class SelectTileDialog : public QDialog
{
	Q_OBJECT

public:
	SelectTileDialog( QWidget* parent = Q_NULLPTR );

	TileInfo getSelectTile() const;

public:
	Ui::TileSelectorUI m_ui;
	QTabWidget* m_tabWidget;
	QList<TilePalette*> m_tilePaletteList;
};