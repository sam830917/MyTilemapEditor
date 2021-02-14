#pragma once

#include "Core/Project.h"
#include "Core/TileInfo.h"
#include <QString>
#include <QSize>
#include <QList>

#define UNUSED(x) (void)(x);
#define STATIC  ;
#define VIRTUAL  ;

class Tileset;
QT_FORWARD_DECLARE_CLASS( QIcon )

enum class eDrawTool
{
	CURSOR,
	MOVE,
	BRUSH,
	ERASER,
	BUCKET,
	MAGIC_WAND,
	SELECT_SAME_TILE,
};
Q_DECLARE_METATYPE( eDrawTool );

QString			getProjectFilePath();
QString			getProjectRootPath();
Project* const	getProject();
TileInfo		getCurrentTile();
QList<TileInfo>	getCurrentTiles();
QSize			getSelectedTilesRegionSize();
QIcon			getApplicationIcon();

void updateProject( Project* newProject );

void saveTilesetIntoProject( Tileset* tileset );
void saveBrushIntoProject( const QString& filePath );
void deleteBrushInProject( const QString& filePath );
void updateBrushFileInProject( const QString& oldFilePath, const QString& newFilePath );

void debugPrint( const QString& text );