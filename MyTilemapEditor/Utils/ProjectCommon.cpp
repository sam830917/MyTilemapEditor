#include "ProjectCommon.h"
#include "Core/Tileset.h"
#include "XmlUtils.h"
#include <QFileInfo>
#include <QDir>

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

void saveBrushIntoProject( const QString& filePath )
{
	if( getProject() == nullptr )
		return;

	XmlDocument* doc = getProject()->getDocument();
	XmlElement* root = doc->RootElement();

	if( !root )
		return;

	XmlElement* brushesEle = root->FirstChildElement( "Brushes" );
	if( !brushesEle )
	{
		brushesEle = doc->NewElement( "Brushes" );
		root->LinkEndChild( brushesEle );
	}

	XmlElement* newBrushEle = doc->NewElement( "Brush" );
	QDir dir( getProjectRootPath() );
	newBrushEle->SetAttribute( "path", dir.relativeFilePath( filePath ).toStdString().c_str() );
	brushesEle->LinkEndChild( newBrushEle );

	saveXmlFile( *doc, getProjectFilePath() );
}

void deleteBrushInProject( const QString& filePath )
{
	XmlDocument* doc = getProject()->getDocument();
	XmlElement* root = doc->RootElement();

	if( !root )
		return;

	XmlElement* brushesEle = root->FirstChildElement( "Brushes" );
	if( !brushesEle )
	{
		return;
	}
	QDir dir( getProjectRootPath() );
	QString oldRelativePath = dir.relativeFilePath( filePath );
	for( XmlElement* brushEle = brushesEle->FirstChildElement( "Brush" ); brushEle; brushEle = brushEle->NextSiblingElement( "Brush" ) )
	{
		QString path = parseXmlAttribute( *brushEle, "path", QString() );
		if( !path.isEmpty() )
		{
			if( oldRelativePath == oldRelativePath )
			{
				brushesEle->DeleteChild( brushEle );
				saveXmlFile( *doc, getProjectFilePath() );
				return;
			}
		}
	}
}

void updateBrushFileInProject( const QString& oldFilePath, const QString& newFilePath )
{
	if( getProject() == nullptr )
		return;

	XmlDocument* doc = getProject()->getDocument();
	XmlElement* root = doc->RootElement();

	if( !root )
		return;

	XmlElement* brushesEle = root->FirstChildElement( "Brushes" );
	if ( !brushesEle )
	{
		return;
	}
	QDir dir( getProjectRootPath() );
	QString oldRelativePath = dir.relativeFilePath( oldFilePath );
	for( XmlElement* brushEle = brushesEle->FirstChildElement( "Brush" ); brushEle; brushEle = brushEle->NextSiblingElement( "Brush" ) )
	{
		QString path = parseXmlAttribute( *brushEle, "path", QString() );
		if ( !path.isEmpty() )
		{
			if ( oldRelativePath == oldRelativePath )
			{
				brushesEle->DeleteChild( brushEle );
				XmlElement* newBrushEle = doc->NewElement( "Brush" );
				newBrushEle->SetAttribute( "path", dir.relativeFilePath( newFilePath ).toStdString().c_str() );
				brushesEle->LinkEndChild( newBrushEle );
				saveXmlFile( *doc, getProjectFilePath() );
				return;
			}
		}
	}
}
