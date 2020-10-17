#pragma once

#include <QString>
#include <QPixmap>

class Tileset
{
public:
	explicit Tileset( QString name, QString imagePath, QPixmap* tilesetImage, QSize tileSize );
	~Tileset();

	void setFilePath( QString path ) { m_filePath = path; }
	void renameFile( const QString& newName );

	QPixmap*	getImage() const { return m_tilesetImage; }
	QSize		getTileSize() const { return m_tileSize; }
	QString		getImagePath() const { return m_imagePath; }
	QString		getFilePath() const { return m_filePath; }
	QString		getRelativeFilePath() const;
	QString		getName() const { return m_name; }

	QSize		getSizeFromIndex( int index ) const;
	QSize		getCoordTileSize() const;
	QPoint		getPoint( int index ) const;

	bool        operator==( const Tileset& compare ) const;
	bool        operator!=( const Tileset& compare ) const;

private:
	QString m_imagePath;
	QString m_filePath;
	QString m_name;
	QPixmap* m_tilesetImage;
	QSize m_tileSize;
};