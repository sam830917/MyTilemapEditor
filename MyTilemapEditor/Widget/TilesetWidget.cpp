#include "TilesetWidget.h"
#include "../Core/Tileset.h"
#include "../Widget/AddTilesetDialog.h"
#include "../Utils/ProjectCommon.h"
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QtMath>
#include <QFileInfo>

TilePalette::TilePalette( QObject* parent /*= Q_NULLPTR*/ )
	:QGraphicsScene( parent )
{
}

void TilePalette::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mousePressEvent( event );
	if( event->button() & Qt::LeftButton )
	{
		QPointF mousePos = event->pos();
		if ( mousePos == QPointF() )
		{
			return;
		}
		QSize size = m_tileset->getTileSize();
		QPoint coord = QPoint( qFloor( mousePos.x() / size.width() ), qFloor( mousePos.y() / size.height() ) );
		//qDebug() << index.x() << "," << index.y();
		
		QPoint startPos = QPoint( coord.x() * size.width(), coord.y() * size.height() );
		m_selectFrame->setRect( startPos.x(), startPos.y(), size.width(), size.height() );
		m_currentIndex = m_tileset->getCoordTileSize().width() * coord.y() + coord.x();
		updateTile( m_tileset, m_currentIndex );
	}
	update();
}

TilesetWidget::TilesetWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_tilesetTabWidget = new QTabWidget( this );
	setWidget( m_tilesetTabWidget );
	connect( m_tilesetTabWidget, &QTabWidget::currentChanged, this, &TilesetWidget::tabCurrentChanged );
}

void TilesetWidget::addTilesetIntoProject( Tileset* tileset )
{
	if ( tileset == nullptr )
	{
		return;
	}
	// check is already exist
	for ( int i = 0; i < m_tilePaletteList.size(); ++i )
	{
		TilePalette* palette = m_tilePaletteList[i];
		if ( tileset->getFilePath() == palette->m_tileset->getFilePath() )
		{
			m_tilesetTabWidget->setCurrentIndex( i );
			return;
		}
	}

	TilePalette* tilePalette = new TilePalette(this);
	tilePalette->m_tileset = tileset;

	m_tilePaletteList.append( tilePalette );

	QGraphicsView* tilesetView = new QGraphicsView();
	tilesetView->setScene( tilePalette );

	QGraphicsRectItem* bg = new QGraphicsRectItem();
	QColor bgColor( 0, 0, 0, 50 );

	bg->setBrush( QBrush( bgColor ) );
	bg->setPen( QPen( bgColor ) );
	bg->setRect( 0, 0, tileset->getImage()->size().width(), tileset->getImage()->size().height() );
	QGraphicsPixmapItem* imgItem = new QGraphicsPixmapItem( *tileset->getImage() );
	tilePalette->addItem( bg );
	tilePalette->addItem( imgItem );

	m_tilesetTabWidget->addTab( tilesetView, tileset->getName() );

	// draw grid
	QSize tileSize = tileset->getTileSize();
	QSize size = tileset->getImage()->size();
	int vLineCount = size.width() / tileSize.width();
	int hLineCount = size.height() / tileSize.height();

	QPen linePen( bgColor );
	linePen.setWidth( 2 );
	for( int v = 1; v < vLineCount; ++v )
	{
		tilePalette->addLine( v * tileSize.width(), 0, v * tileSize.width(), size.height(), linePen );
	}
	for( int h = 1; h < hLineCount; ++h )
	{
		tilePalette->addLine( 0, h * tileSize.height(), size.width(), h * tileSize.height(), linePen );
	}

	tilePalette->m_selectFrame = new QGraphicsRectItem();
	QBrush brush( QColor( 0, 0, 255, 70 ) );
	QPen redPen( Qt::red );
	redPen.setWidth( 2 );
	tilePalette->m_selectFrame->setRect( 0, 0, tileset->getTileSize().width(), tileset->getTileSize().height() );
	tilePalette->m_selectFrame->setBrush( brush );
	tilePalette->m_selectFrame->setPen( redPen );
	tilePalette->addItem( tilePalette->m_selectFrame );
}

void TilesetWidget::tabCurrentChanged( int index )
{
	TilePalette* p = m_tilePaletteList[index];
	updateTile( p->m_tileset, p->m_currentIndex );
}

void TilesetWidget::addTileset()
{
	AddTilesetDialog dialog( this );
	if( dialog.exec() == QDialog::Accepted )
	{
		Tileset* newTileset = dialog.getResult();
		if( newTileset == Q_NULLPTR )
		{
			return;
		}
		addTilesetIntoProject( newTileset );
		saveTilesetIntoProject( newTileset );
	}
}

void TilesetWidget::tilesetRenamed( const QString& path, const QString& oldName, const QString& newName )
{
	for ( int i = 0; i < m_tilePaletteList.size(); i++ )
	{
		TilePalette* tilepalette = m_tilePaletteList[i];
		if ( tilepalette->m_tileset->getName() == QFileInfo(oldName).completeBaseName() )
		{
			QString baseName = QFileInfo( newName ).completeBaseName();
			tilepalette->m_tileset->renameFile( baseName );
			m_tilesetTabWidget->setTabText( i, baseName );
		}
	}
}
