#pragma once

#include "Core/Project.h"
#include "Core/TileInfo.h"
#include <QString>

#define UNUSED(x) (void)(x);
#define STATIC  ;
#define VIRTUAL  ;

class Tileset;

QString			getProjectFilePath();
QString			getProjectRootPath();
Project* const	getProject();
TileInfo		getCurrentTile();

void updateProject( Project* newProject );
void updateTile( const Tileset* tileset, int index );

void saveTilesetIntoProject( Tileset* tileset );
void saveBrushIntoProject( const QString& filePath );
void updateBrushFileInProject( const QString& oldFilePath, const QString& newFilePath );