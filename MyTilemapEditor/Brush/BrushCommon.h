#pragma once

#include <QString>
#include <QBoxLayout>
#include <QTreeWidgetItem>
#include <QList>
#include "Core/TileInfo.h"

enum class eItemType
{
	STRING,
	INT,
	TILE_INFO,
	TILE_INFO_LIST,
	TILE_GRID_BOOL,
	TILE_GRID_BOOL_LIST_EDGE,
	TILE_GRID_BOOL_LIST_CORNER,

	UNKNOWN,
};

struct TileModified
{
	TileModified( QPoint coordinate, TileInfo tileInfo ):m_coordinate( coordinate ), m_tileInfo( tileInfo ) {}

	QPoint m_coordinate;
	TileInfo m_tileInfo;
};

struct AddBrushItem
{
	eItemType m_type = eItemType::UNKNOWN;
	QString m_name;
	QWidget* m_widgetItem;
	QTreeWidgetItem* m_treeItem;
};

bool isListType( eItemType type );