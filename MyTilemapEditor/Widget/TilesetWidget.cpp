#include "TilesetWidget.h"
#include "Core/Tileset.h"
#include "Widget/AddTilesetDialog.h"
#include "Utils/ProjectCommon.h"
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QtMath>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollBar>

QList<Tileset*> g_tilesetList;
extern QList<TileInfo> g_currentTileInfos;
extern QSize g_selectedTilesRegionSize;

TilePalette::TilePalette( Tileset* tileset, bool isMainPalette, QObject* parent /*= Q_NULLPTR*/ )
	:QGraphicsScene( parent ),
	m_isMainPalette(isMainPalette)
{
	QGraphicsRectItem* bg = new QGraphicsRectItem();
	QColor bgColor( 0, 0, 0, 50 );

	bg->setBrush( QBrush( bgColor ) );
	bg->setPen( QPen( bgColor ) );
	bg->setRect( 0, 0, tileset->getImage()->size().width(), tileset->getImage()->size().height() );
	QGraphicsPixmapItem* imgItem = new QGraphicsPixmapItem( *tileset->getImage() );
	addItem( bg );
	addItem( imgItem );

	// draw grid
	QSize tileSize = tileset->getTileSize();
	QSize size = tileset->getImage()->size();
	int vLineCount = size.width() / tileSize.width();
	int hLineCount = size.height() / tileSize.height();

	QPen linePen( bgColor );
	linePen.setWidth( 2 );
	for( int v = 1; v < vLineCount; ++v )
	{
		addLine( v * tileSize.width(), 0, v * tileSize.width(), size.height(), linePen );
	}
	for( int h = 1; h < hLineCount; ++h )
	{
		addLine( 0, h * tileSize.height(), size.width(), h * tileSize.height(), linePen );
	}

	m_selectFrame = new QGraphicsRectItem();
	QBrush brush( QColor( 0, 0, 255, 70 ) );
	QPen redPen( Qt::red );
	redPen.setWidth( 2 );
	m_selectFrame->setRect( 0, 0, tileset->getTileSize().width(), tileset->getTileSize().height() );
	m_selectFrame->setBrush( brush );
	m_selectFrame->setPen( redPen );
	addItem( m_selectFrame );
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
		m_startPos = mousePos;
// 		qDebug() << m_startPos.x() << "," << m_startPos.y();
		
		QPoint startPos = QPoint( coord.x() * size.width(), coord.y() * size.height() );
		m_selectFrame->setRect( startPos.x(), startPos.y(), size.width(), size.height() );
		m_selectedRegionSize = QSize( 1, 1 );
// 		m_selectedIndexes.clear();
// 		m_selectedIndexes.insert( m_tileset->getTileIndex( coord ) );
// 		m_currentIndex = m_tileset->getTileIndex( coord );
		if ( m_isMainPalette )
		{
			g_selectedTilesRegionSize = m_selectedRegionSize;
			g_currentTileInfos.clear();
			g_currentTileInfos.push_back( TileInfo( m_tileset, m_tileset->getTileIndex( coord ) ) );
			m_selectedIndexes.clear();
			m_selectedIndexes.push_back( TileInfo( m_tileset, m_tileset->getTileIndex( coord ) ) );
// 			updateTile( m_tileset, m_currentIndex );
		}
	}
	update();
}

void TilePalette::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mouseMoveEvent( event );
	if( event->buttons() & Qt::LeftButton )
	{
		QPointF mousePos = event->scenePos();
		if( mousePos == QPointF() )
		{
			return;
		}
		QSize size = m_tileset->getTileSize();
		QPointF minPoint = QPointF( qMin( m_startPos.x(), mousePos.x() ), qMin( m_startPos.y(), mousePos.y() ) );
		QPointF maxPoint = QPointF( qMax( m_startPos.x(), mousePos.x() ), qMax( m_startPos.y(), mousePos.y() ) );
		QPoint selectedMinCoord = QPoint( qMax( 0, qFloor( minPoint.x() / size.width() )), qMax( 0, qFloor( minPoint.y() / size.height() ) ) );
		QPoint selectedMaxCoord = QPoint( qMin( size.width(), qCeil( maxPoint.x() / size.width() ) ), qMin( size.height(), qCeil( maxPoint.y() / size.height() ) ) );
		QPoint startPos = QPoint( selectedMinCoord.x() * size.width(), selectedMinCoord.y() * size.height() );
		QPoint endPos = QPoint( selectedMaxCoord.x() * size.width(), selectedMaxCoord.y() * size.height() );

		QSize regionSize = QSize( endPos.x() - startPos.x(), endPos.y() - startPos.y() );
		m_selectFrame->setRect( startPos.x(), startPos.y(), regionSize.width(), regionSize.height() );

		if( m_isMainPalette )
		{
			g_currentTileInfos.clear();
			m_selectedIndexes.clear();
			QSize regionSizeInCoord = QSize( selectedMaxCoord.x() - selectedMinCoord.x(), selectedMaxCoord.y() - selectedMinCoord.y() );
			m_selectedRegionSize = regionSizeInCoord;
			g_selectedTilesRegionSize = m_selectedRegionSize;
			
			for( int y = 0; y < regionSizeInCoord.height(); ++y )
			{
				for( int x = 0; x < regionSizeInCoord.width(); ++x )
				{
					g_currentTileInfos.push_back( TileInfo( m_tileset, m_tileset->getTileIndex( selectedMinCoord + QPoint( x, y ) ) ) );
					m_selectedIndexes.push_back( TileInfo( m_tileset, m_tileset->getTileIndex( selectedMinCoord + QPoint( x, y ) ) ) );
				}
			}
		}
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

	TilePalette* tilePalette = new TilePalette( tileset, true, this);
	tilePalette->m_tileset = tileset;

	m_tilePaletteList.append( tilePalette );
	g_tilesetList.append( tileset );

	QGraphicsView* tilesetView = new QGraphicsView();
	tilesetView->setScene( tilePalette );

	int tabIndex = m_tilesetTabWidget->addTab( tilesetView, tileset->getName() );
	m_tilesetTabWidget->setTabIcon( tabIndex, QIcon( ":/MainWindow/Icon/tileset_icon.png" ) );
	tilesetView->horizontalScrollBar()->setSliderPosition( tilesetView->horizontalScrollBar()->minimum() );
	tilesetView->verticalScrollBar()->setSliderPosition( tilesetView->verticalScrollBar()->minimum() );
}

void TilesetWidget::tabCurrentChanged( int index )
{
	if ( index < 0 )
	{
		return;
	}
	TilePalette* p = m_tilePaletteList[index];
	g_currentTileInfos.clear();
	for ( TileInfo t : p->m_selectedIndexes )
	{
		g_currentTileInfos.push_back( t );
	}
}

void TilesetWidget::addTileset()
{
	AddTilesetDialog dialog( this );
	if( dialog.exec() == QDialog::Accepted )
	{
		Tileset* newTileset = dialog.getResult();
		if( newTileset == Q_NULLPTR )
		{
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Tileset File." ) );
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

void TilesetWidget::closeAllTab()
{
	int count = m_tilesetTabWidget->count();
	for( int i = 0; i < count; ++i )
	{
		m_tilesetTabWidget->removeTab( i );
	}
	m_tilesetTabWidget->clear();

	for( int i = 0; i < m_tilePaletteList.size(); ++i )
	{
		delete m_tilePaletteList[i];
		delete g_tilesetList[i];
	}
	m_tilePaletteList.clear();
	g_tilesetList.clear();
	g_currentTileInfos.clear();
}
