#pragma once

#include "../ThirdParty/tinyxml2.h"
#include <QString>
#include <QSize>

class Tileset;
class MapInfo;

typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLAttribute XmlAttribute;

int			parseXmlAttribute( const XmlElement& element, const char* attributeName, int defaultValue );
char		parseXmlAttribute( const XmlElement& element, const char* attributeName, char defaultValue );
bool		parseXmlAttribute( const XmlElement& element, const char* attributeName, bool defaultValue );
float		parseXmlAttribute( const XmlElement& element, const char* attributeName, float defaultValue );
QString		parseXmlAttribute( const XmlElement& element, const char* attributeName, QString defaultValue );
QSize		parseXmlAttribute( const XmlElement& element, const char* attributeName, QSize defaultValue );

void		saveXmlFile( XmlDocument& doc, const QString& filePath );

// convert functions
Tileset*	convertToTileset( const QString& tilesetFilePath );
MapInfo*	convertToMapInfo( const QString& mapFilePath );