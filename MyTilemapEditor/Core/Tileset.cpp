#include "Tileset.h"
#include "Utils/ProjectCommon.h"
#include "Utils/XmlUtils.h"
#include <QDir>
#include <QFileInfo>

Tileset::Tileset( QString name, QString imagePath, QPixmap* tilesetImage, QSize tileSize )
	:m_name(name),
	m_imagePath(imagePath),
	m_tilesetImage(tilesetImage),
	m_tileSize(tileSize)
{
}

Tileset::~Tileset()
{
}

void Tileset::renameFile( const QString& newName )
{
	m_name = newName;
	QFileInfo oldNameFileInfo(m_filePath);
	m_filePath = oldNameFileInfo.path() + "/" + newName + ".tileset";

	XmlDocument* doc = new XmlDocument();
	doc->LoadFile( m_filePath.toStdString().c_str() );
	if( doc->Error() )
	{
		return;
	}
	XmlElement* root = doc->RootElement();
	XmlElement* imgEle = root->FirstChildElement( "Image" );
	if ( imgEle )
	{
		QString path = parseXmlAttribute( *imgEle, "path", QString() );
		if ( path.isEmpty() )
		{
			return;
		}

		QDir f( oldNameFileInfo.path() );
		QString absoluteImagePath = f.absoluteFilePath( path );
		QFile imgFile(absoluteImagePath);
		if ( imgFile.exists() )
		{
			QString newPath = oldNameFileInfo.path() + "/" + newName + "." + QFileInfo(path).suffix();
			imgFile.rename( newPath );

			imgEle->SetAttribute( "path", f.relativeFilePath(newPath).toStdString().c_str() );
			saveXmlFile( *doc, m_filePath );
		}
	}
}

QString Tileset::getRelativeFilePath() const
{
	QDir dir( getProjectRootPath() );

	return dir.relativeFilePath( m_filePath );
}

QSize Tileset::getSizeFromIndex( int index ) const
{
	QSize imageSize = getCoordTileSize();
	return QSize( index % imageSize.width(), index / imageSize.width() );
}

QSize Tileset::getCoordTileSize() const
{
	QSize imagePixelSize = m_tilesetImage->size();
	return QSize( imagePixelSize.width() / m_tileSize.width(), imagePixelSize.height() / m_tileSize.height() );
}

QPoint Tileset::getPoint( int index ) const
{
	QSize tileSize = getSizeFromIndex( index );
	return QPoint( tileSize.width() * m_tileSize.width(), tileSize.height() * m_tileSize.height() );
}

int Tileset::getTileIndex( const QPoint& coord ) const
{
	return getCoordTileSize().width() * coord.y() + coord.x();
}

bool Tileset::operator==( const Tileset* compare ) const
{
	return *this == *compare;
}

bool Tileset::operator!=( const Tileset& compare ) const
{
	return m_filePath != compare.m_filePath;
}

bool Tileset::operator==( const Tileset& compare ) const
{
	return m_filePath == compare.m_filePath;
}
