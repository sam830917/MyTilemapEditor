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

	TilePalette* tilePalette = new TilePalette( tileset, this);

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
	p->updateGlobalTilePalette();
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
	g_selectedTilesRegionSize = QSize();
}
