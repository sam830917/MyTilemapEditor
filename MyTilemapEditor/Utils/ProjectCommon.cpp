#include "ProjectCommon.h"
#include "Core/Tileset.h"
#include "XmlUtils.h"
#include <QFileInfo>

static Project* g_project = nullptr;
static TileInfo g_currentTileInfo;

QString getProjectFilePath()
{
	if ( g_project == nullptr )
	{
		return QString();
	}

	return g_project->getFilePath();
}

QString getProjectRootPath()
{
	if( getProject() == nullptr )
	{
		return QString();
	}

	QFileInfo fileInfo( g_project->getFilePath() );
	return fileInfo.path();
}

Project* const getProject()
{
	return g_project;
}

TileInfo getCurrentTile()
{
	return g_currentTileInfo;
}

void updateProject( Project* newProject )
{
	g_project = newProject;
}

void updateTile( const Tileset* tileset, int index )
{
	g_currentTileInfo = TileInfo( tileset, index );
}

void saveTilesetIntoProject( Tileset* tileset )
{
	if ( getProject() == nullptr )
		return;

	XmlDocument* doc = getProject()->getDocument();
	XmlElement* root = doc->RootElement();

	if ( !root )
		return;

	XmlElement* tilesetEle = root->FirstChildElement( "Tilesets" );
	if ( !tilesetEle )
	{
		tilesetEle = doc->NewElement( "Tilesets" );
		root->LinkEndChild( tilesetEle );
	}

	XmlElement* newTilesetEle = doc->NewElement( "Tileset" );
	newTilesetEle->SetAttribute( "path", tileset->getRelativeFilePath().toStdString().c_str() );
	tilesetEle->LinkEndChild( newTilesetEle );

	saveXmlFile( *doc, getProjectFilePath() );
}
