#include "Widget/SelectTileDialog.h"
#include "Utils/ProjectCommon.h"
#include <QTabWidget>
#include <QGraphicsView>
#include <QScrollBar>
#include <QIcon>

extern QList<Tileset*> g_tilesetList;

SelectTileDialog::SelectTileDialog( QWidget* parent /*= Q_NULLPTR */ )
{
	m_ui.setupUi( this );
	setWindowIcon( getApplicationIcon() );
	setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint );
	m_tabWidget = new QTabWidget(this);
	m_ui.horizontalLayout->addWidget(m_tabWidget);

	for ( Tileset* tileset : g_tilesetList )
	{
		TilePalette* tilePalette = new TilePalette( tileset, this );
		tilePalette->m_updateGlobalTilePalette = false;
		QGraphicsView* tilesetView = new QGraphicsView();
		tilesetView->setScene( tilePalette );
		m_tilePaletteList.push_back( tilePalette );
		tilesetView->verticalScrollBar()->setSliderPosition( 0 );
		tilesetView->horizontalScrollBar()->setSliderPosition( 0 );

		m_tabWidget->addTab( tilesetView, tileset->getName() );
		tilesetView->horizontalScrollBar()->setSliderPosition( tilesetView->horizontalScrollBar()->minimum() );
		tilesetView->verticalScrollBar()->setSliderPosition( tilesetView->verticalScrollBar()->minimum() );
	}
	setSelectTileMode( ePaletteSelectMode::PALETTE_SINGLE_SELECT );
}

SelectTileDialog::~SelectTileDialog()
{
	for ( int i = 0; i < m_tilePaletteList.size(); ++i )
	{
		delete m_tilePaletteList[i];
		m_tilePaletteList[i] = nullptr;
	}
	delete m_tabWidget;
	m_tabWidget = nullptr;
}

TileInfo SelectTileDialog::getSelectSingleTile() const
{
	int index = m_tilePaletteList[m_tabWidget->currentIndex()]->m_currentIndex;
	return TileInfo( m_tilePaletteList[m_tabWidget->currentIndex()]->m_tileset, index );
}

QList<TileInfo> SelectTileDialog::getSelectTiles() const
{
	return m_tilePaletteList[m_tabWidget->currentIndex()]->m_selectedTileInfos;
}

QSize SelectTileDialog::getSelectTileRegion() const
{
	return m_tilePaletteList[m_tabWidget->currentIndex()]->m_selectedRegionSize;
}

void SelectTileDialog::setSelectTileMode( ePaletteSelectMode selectMode )
{
	for( int i = 0; i < m_tilePaletteList.size(); ++i )
	{
		m_tilePaletteList[i]->m_selectMode = selectMode;
	}
}
