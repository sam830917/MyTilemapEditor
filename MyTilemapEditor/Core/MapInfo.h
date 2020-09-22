#pragma once

#include <QSize>
#include <QString>

class MapInfo
{
public:
	MapInfo();
	MapInfo( int mapWidth, int mapHeight, int tileWidth, int tileHeight );
	MapInfo( const QSize& mapSize, const QSize& tileSize );
	~MapInfo();

	void setFilePath( const QString& filePath ) { m_filePath = filePath; }
	void setName( const QString& name ) { m_name = name; }

	QString		getFilePath() const { return m_filePath; }
	QString		getName() const { return m_name; }
	QSize		getMapSize() const { return QSize( m_mapWidth, m_mapHeight ); }
	QSize		getTileSize() const { return QSize( m_tileWidth, m_tileHeight ); }

private:
	QString m_filePath;
	QString m_name;
	int m_mapHeight = 1;
	int m_mapWidth = 1;
	int m_tileHeight = 32;
	int m_tileWidth = 32;

};