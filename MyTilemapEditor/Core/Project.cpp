#include "Project.h"
#include <QFileInfo>
#include <QDir>

Project::Project( XmlDocument* projectXml, const QString& filePath )
	:m_projectXml( projectXml ),
	m_filePath( filePath )
{
}

Project::~Project()
{

}

QStringList Project::getDirectories()
{
	QStringList pathList;
	if ( m_projectXml )
	{
		XmlElement* root = m_projectXml->RootElement();

		XmlElement* dirEle = root->FirstChildElement( "Directories" );
		if ( dirEle )
		{
			for ( XmlElement* childEle = dirEle->FirstChildElement("Directory"); childEle != nullptr; childEle = childEle->NextSiblingElement("Directory") )
			{
				QString path = parseXmlAttribute( *childEle, "path", QString(",") );
				if ( path == "," )
				{
					QFileInfo file(m_filePath);
					pathList<<file.path();
				}
				else
				{
					pathList<<path;
				}
			}
		}
	}

	return pathList;
}

QString Project::getName() const
{
	QFileInfo info(m_filePath);
	return info.baseName();
}

void Project::renameTilesetFile( const QString& oldNamePath, const QString& newNamePath )
{
	XmlElement* root = m_projectXml->RootElement();

	XmlElement* tilesetEle = root->FirstChildElement( "Tilesets" );
	if ( tilesetEle )
	{
		for( XmlElement* childEle = tilesetEle->FirstChildElement( "Tileset" ); childEle != nullptr; childEle = childEle->NextSiblingElement( "Tileset" ) )
		{
			QString path = parseXmlAttribute( *childEle, "path", QString( "" ) );
			if ( path.isEmpty() )
			{
				continue;
			}
			QFileInfo info(m_filePath);
			QDir f(info.path());
			QString absolutePath = f.absoluteFilePath(path);
			if ( QFileInfo(absolutePath) == QFileInfo(oldNamePath) )
			{
				QDir fileFolderPath(info.path());
				childEle->SetAttribute( "path", fileFolderPath.relativeFilePath(newNamePath).toStdString().c_str() );
				saveXmlFile( *m_projectXml, m_filePath );
				return;
			}
		}
	}
}
