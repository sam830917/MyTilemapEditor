#include "Brush/BrushCommon.h"
#include "Core/TileInfo.h"
#include "Core/Tileset.h"
#include "Core/TileSelector.h"
#include "Core/TileInfoListContainer.h"
#include "Core//IntInput.h"
#include "Utils/XmlUtils.h"
#include "Utils/ProjectCommon.h"
#include <QLineEdit>
#include <QPushButton>

bool isListType( eItemType type )
{
	return type == eItemType::TILE_INFO_LIST || type == eItemType::TILE_GRID_BOOL_LIST_EDGE;
}
