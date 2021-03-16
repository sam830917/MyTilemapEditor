#include "BrushUI/TileSelector.h"
#include "Widget/SelectTileDialog.h"
#include <QGraphicsRectItem>

class TileSelectorScene : public QGraphicsScene
{
	friend class TileSelector;

public:
	TileSelectorScene( const QSize& size );

	void setTileInfo( TileInfo tileinfo );

private:
	TileSelector* m_parentView;
	TileItem* m_image;
};

TileSelectorScene::TileSelectorScene( const QSize& size )
	:QGraphicsScene()
{
	TileItem* image = new TileItem( size );
	addItem( image );
	m_image = image;
}

void TileSelectorScene::setTileInfo( TileInfo tileinfo )
{
	m_image->setTileInfo(tileinfo);
	m_parentView->tileChanged( tileinfo );
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
	delete m_scene;
	m_scene = nullptr;
}

TileInfo TileSelector::getTileinfo() const
{
	return m_scene->m_image->getTileInfo();
}

void TileSelector::setTileInfo( TileInfo tileinfo )
{
	m_scene->setTileInfo( tileinfo );
}

//----------------------------------------------------------------------------------------------------
TileItem::TileItem( const QSize& size, const QPointF& position )
	:QGraphicsRectItem(),
	m_size( size ),
	m_position(position)
{
	setBrush( QBrush( QColor( 255, 0, 255, 255 ) ) );
	setRect( position.x(), position.y(), m_size.width(), m_size.height() );
}

TileItem::~TileItem()
{

}

void TileItem::setTileInfo( TileInfo tileInfo )
{
	m_selectedTile = tileInfo;
	update();
}

void TileItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= Q_NULLPTR */ )
{
	if ( m_selectedTile.isValid() )
	{
		QPixmap img = m_selectedTile.getTileImage();
		painter->drawPixmap( m_position.x(), m_position.y(), m_size.width(), m_size.height(), img );
	}
	else
	{
		painter->setBrush( QBrush( QColor( 255, 0, 255, 255 ) ) );
		painter->drawRect( m_position.x(), m_position.y(), m_size.width(), m_size.height() );
	}
}

void TileItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	SelectTileDialog dialog;
	if( dialog.exec() == QDialog::Accepted )
	{
		TileInfo tileinfo = dialog.getSelectSingleTile();
		setTileInfo( tileinfo );
		update();
	}
	QGraphicsItem::mousePressEvent(event);
}
