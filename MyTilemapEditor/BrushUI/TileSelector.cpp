#include "BrushUI/TileSelector.h"
#include "Widget/SelectTileDialog.h"
#include <QGraphicsRectItem>

class TileSelectorScene : public QGraphicsScene
{
	friend class TileSelector;

public:
	TileSelectorScene( const QSize& size );

	void setTileInfo( TileInfo tileinfo );

protected:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	TileSelector* m_parentView;
	QGraphicsItem* m_image;
};

TileSelectorScene::TileSelectorScene( const QSize& size )
	:QGraphicsScene()
{
	QGraphicsRectItem* image = new QGraphicsRectItem();
	image->setRect( 0, 0, size.width(), size.height() );
	image->setBrush( QBrush( QColor( 255, 0, 255, 255 ) ) );
	addItem( image );
	m_image = image;
}

void TileSelectorScene::setTileInfo( TileInfo tileinfo )
{
	if ( tileinfo.isValid() )
	{
		if ( m_image )
		{
			delete m_image;
			m_image = nullptr;
		}
		QPixmap img = tileinfo.getTileImage();
		m_image = addPixmap( img.scaled( m_parentView->getSize().width(), m_parentView->getSize().height(), Qt::KeepAspectRatio ) );
		m_parentView->m_selectedTile = tileinfo;
	}
	else
	{
		QGraphicsRectItem* image = new QGraphicsRectItem();
		image->setRect( 0, 0, m_parentView->getSize().width(), m_parentView->getSize().height() );
		image->setBrush( QBrush( QColor( 255, 0, 255, 255 ) ) );
		addItem( image );
		m_image = image;
	}
	m_parentView->tileChanged( tileinfo );
}

void TileSelectorScene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	SelectTileDialog dialog;

	if( dialog.exec() == QDialog::Accepted )
	{
		TileInfo tileinfo = dialog.getSelectTile();
		delete m_image;
		m_image = nullptr;
		setTileInfo( tileinfo );
	}
}

//----------------------------------------------------------------------------------------------------
TileSelector::TileSelector( const QSize& size )
	:QGraphicsView(),
	m_size(size)
{
	setMaximumSize( size );
	setMinimumSize( size );
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

	m_scene = new TileSelectorScene( size );
	setScene(m_scene);
	m_scene->m_parentView = this;
}

TileSelector::~TileSelector()
{
}

void TileSelector::setTileInfo( TileInfo tileinfo )
{
	m_selectedTile = tileinfo;
	m_scene->setTileInfo( tileinfo );
}
