#pragma once

#include <QSize>
#include <QString>
#include <QPoint>

class MapInfo
{
public:
	MapInfo();
	MapInfo( int mapWidth, int mapHeight, int tileWidth, int tileHeight );
	MapInfo( const QSize& mapSize, const QSize& tileSize );
	~MapInfo();

	void setFilePath( const QString& filePath ) { m_filePath = filePath; }
	void setName( const QString& name ) { m_name = name; }
	void setIsValid( bool isValid ) { m_isValid = isValid; }
	bool isOutOfBound( const QPoint& coord );
	int  getIndex( const QPoint& coord ) const { return coord.y() * getMapSize().width() + coord.x(); }
	QPoint getCoord( int index ) const { return QPoint( index % m_mapWidth, index / m_mapWidth ); }

	QString		getFilePath() const { return m_filePath; }
	QString		getName() const { return m_name; }
	QSize		getMapSize() const { return QSize( m_mapWidth, m_mapHeight ); }
	QSize		getTileSize() const { return QSize( m_tileWidth, m_tileHeight ); }
	bool		IsValid() const { return m_isValid; }

private:
	QString m_filePath;
	QString m_name;
	int m_mapHeight = 1;
	int m_mapWidth = 1;
	int m_tileHeight = 32;
	int m_tileWidth = 32;
	bool m_isValid = false;
};