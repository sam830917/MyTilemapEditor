#pragma once

#include "Core/Project.h"
#include "Core/TileInfo.h"
#include <QString>

class Tileset;

QString			getProjectFilePath();
QString			getProjectRootPath();
Project* const	getProject();
TileInfo		getCurrentTile();

void updateProject( Project* newProject );
void updateTile( const Tileset* tileset, int index );

void saveTilesetIntoProject( Tileset* tileset );