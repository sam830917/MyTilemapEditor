#include "Brush/BrushCommon.h"
#include "Core/TileInfo.h"
#include "Core/Tileset.h"
#include "Utils/XmlUtils.h"
#include "Utils/ProjectCommon.h"
#include <QLineEdit>
#include <QPushButton>

uint qHash( const TileModified key )
{
	return key.m_coordinate.x() + key.m_coordinate.y();
}

bool isListType( eItemType type )
{
	return type == eItemType::TILE_INFO_LIST || type == eItemType::TILE_GRID_BOOL_LIST_EDGE || type == eItemType::TILE_GRID_BOOL_LIST_CORNER;
}

bool TileModified::operator==( const TileModified& compare ) const
{
	return compare.m_coordinate.x() == m_coordinate.x() && compare.m_coordinate.y() == m_coordinate.y() && compare.m_tileInfo == m_tileInfo;
}
