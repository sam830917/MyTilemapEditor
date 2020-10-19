#include "Widget/SelectTileDialog.h"
#include "Widget/TilesetWidget.h"
#include <QTabWidget>
#include <QGraphicsView>
#include <QScrollBar>

extern QList<Tileset*> g_tilesetList;

SelectTileDialog::SelectTileDialog( QWidget* parent /*= Q_NULLPTR */ )
{
	m_ui.setupUi( this );
	m_tabWidget = new QTabWidget(this);
	m_ui.horizontalLayout->addWidget(m_tabWidget);

	for ( Tileset* tileset : g_tilesetList )
	{
		TilePalette* tilePalette = new TilePalette( tileset, false, this );
		tilePalette->m_tileset = tileset;
		QGraphicsView* tilesetView = new QGraphicsView();
		tilesetView->setScene( tilePalette );
		m_tilePaletteList.push_back( tilePalette );
		tilesetView->verticalScrollBar()->setSliderPosition( 0 );
		tilesetView->horizontalScrollBar()->setSliderPosition( 0 );

		m_tabWidget->addTab( tilesetView, tileset->getName() );
		tilesetView->horizontalScrollBar()->setSliderPosition( tilesetView->horizontalScrollBar()->minimum() );
		tilesetView->verticalScrollBar()->setSliderPosition( tilesetView->verticalScrollBar()->minimum() );
	}
}

TileInfo SelectTileDialog::getSelectTile() const
{
	int index = m_tilePaletteList[m_tabWidget->currentIndex()]->m_currentIndex;
	return TileInfo( m_tilePaletteList[m_tabWidget->currentIndex()]->m_tileset, index );
}
