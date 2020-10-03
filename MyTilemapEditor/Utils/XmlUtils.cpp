#include "XmlUtils.h"
#include "ProjectCommon.h"
#include "Core/Tileset.h"
#include "Core/MapInfo.h"
#include "Core/LayerInfo.h"
#include <string>
#include <QFile>
#include <QMessageBox>
#include <QFileInfo>
#include <QPixmap>

int parseXmlAttribute( const XmlElement& element, const char* attributeName, int defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	int value = defaultValue;
	if( attributeValueText )
	{
		value = atoi( attributeValueText );
	}

	return value;
}

char parseXmlAttribute( const XmlElement& element, const char* attributeName, char defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	char value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText[0];
	}

	return value;
}

bool parseXmlAttribute( const XmlElement& element, const char* attributeName, bool defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	bool value = defaultValue;
	if( attributeValueText )
	{
		std::string str = attributeValueText;
		if( str == "true" )
		{
			value = true;
		}
		else if( str == "false" )
		{
			value = false;
		}
	}

	return value;
}

float parseXmlAttribute( const XmlElement& element, const char* attributeName, float defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	float value = defaultValue;
	if( attributeValueText )
	{
		value = (float)atof( attributeValueText );
	}

	return value;
}

QString parseXmlAttribute( const XmlElement& element, const char* attributeName, QString defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	QString value(defaultValue);
	if( attributeValueText )
	{
		value = attributeValueText;
	}

	return value;
}

QSize parseXmlAttribute( const XmlElement& element, const char* attributeName, QSize defaultValue )
{
	QString str = parseXmlAttribute( element, attributeName, QString() );
	if ( str.isEmpty() )
	{
		return defaultValue;
	}
	QSize value(defaultValue);
	QStringList strList = str.split(",");
	if ( strList.size() == 2 )
	{
		value = QSize( strList[0].toInt(), strList[1].toInt() );
	}

	return value;
}

void saveXmlFile( XmlDocument& doc, const QString& filePath )
{
	std::string fileNameStr = filePath.toStdString();
	doc.SaveFile( fileNameStr.c_str() );
}

Tileset* convertToTileset( const QString& tilesetFilePath )
{
	QFile file( tilesetFilePath );
	if( !file.exists() )
	{
		return nullptr;
	}
	XmlDocument* doc = new XmlDocument;
	doc->LoadFile( tilesetFilePath.toStdString().c_str() );
	if ( doc->Error() )
	{
		return nullptr;
	}
	XmlElement* root = doc->RootElement();
	if ( !root )
	{
		return nullptr;
	}
	XmlElement* imageEle = root->FirstChildElement( "Image" );
	if ( imageEle )
	{
		QFileInfo fileInfo(tilesetFilePath);

		QSize size = parseXmlAttribute( *imageEle, "size", QSize(32, 32) );
		QString imageRelativePath = parseXmlAttribute( *imageEle, "path", QString() );
		QString imagePath = getProjectRootPath() + "/" + imageRelativePath;
		QPixmap* tilesetImage = new QPixmap();
		if( !(tilesetImage->load( imagePath )) )
		{
			delete tilesetImage;
			return nullptr;
		}

		Tileset* t = new Tileset( fileInfo.completeBaseName(), imagePath, tilesetImage, size );
		t->setFilePath(tilesetFilePath);
		return t;
	}

	return nullptr;
}

void convertToMapInfo( const QString& mapFilePath, MapInfo& mapInfo, QList<LayerInfo>& layerInfo )
{
	QFile file( mapFilePath );
	if( !file.exists() )
	{
		return;
	}
	XmlDocument* doc = new XmlDocument;
	doc->LoadFile( mapFilePath.toStdString().c_str() );
	if( doc->Error() )
	{
		return;
	}
	XmlElement* root = doc->RootElement();
	if( root )
	{
		QFileInfo fileInfo( mapFilePath );

		QSize mapSize = parseXmlAttribute( *root, "mapSize", QSize( 30, 30 ) );
		QSize tileSize = parseXmlAttribute( *root, "tileSize", QSize( 32, 32 ) );
		mapInfo = MapInfo( mapSize, tileSize );
		mapInfo.setFilePath( mapFilePath );
		mapInfo.setName( fileInfo.completeBaseName() );
		mapInfo.setIsValid( true );

		// Load Layers
		XmlElement* layersEle = root->FirstChildElement( "Layers" );
		if ( layersEle )
		{
			LayerInfo emptyInfo;
			for( XmlElement* layerEle = layersEle->FirstChildElement( "Layer" ); layerEle; layerEle = layerEle->NextSiblingElement( "Layer" ) )
			{
				QString name = parseXmlAttribute( *layerEle, "name", emptyInfo.getNmae() );
				bool isLock = parseXmlAttribute( *layerEle, "isLock", emptyInfo.IsLock() );
				bool isVisible = parseXmlAttribute( *layerEle, "isVisible", emptyInfo.IsVisible() );

				layerInfo.push_back( LayerInfo( name, isLock, isVisible ) );
			}
		}
		else
		{
			layerInfo.push_back( LayerInfo() );
		}
	}

	return;
}
