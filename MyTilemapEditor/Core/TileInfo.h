#pragma once

#include <QObject>
#include <QPixmap>

class Tileset;

class TileInfo : public QObject
{
	Q_OBJECT
public:
	Q_INVOKABLE TileInfo();
	TileInfo( const TileInfo& copyFrom );
	TileInfo( const Tileset* tileset, int index );
	~TileInfo();

	QPixmap			getTileImage();
	int				getIndex() const { return m_index; }
	const Tileset*	getTileset() const { return m_tileset; }

	void setTileInfo( const Tileset* tileset, int index ) { m_tileset = tileset; m_index = index; }

	Q_INVOKABLE bool	isValid() const { return m_index != -1 && m_index >=0 && m_tileset; }

	Q_INVOKABLE void        operator=( const TileInfo& compare );
	Q_INVOKABLE bool        operator==( const TileInfo& compare ) const;
	Q_INVOKABLE bool        operator!=( const TileInfo& compare ) const;

private:
	const Tileset* m_tileset = nullptr;
	int m_index = -1;
};