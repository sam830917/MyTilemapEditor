#include "Core/TileSelector.h"
#include "Widget/SelectTileDialog.h"
#include <QGraphicsRectItem>

class TileSelectorScene : public QGraphicsScene
{
	friend class TileSelector;

public:
	TileSelectorScene( const QSize& size );

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
	addItem( image );
	m_image = image;
}

void TileSelectorScene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	SelectTileDialog dialog;

	if( dialog.exec() == QDialog::Accepted )
	{
		TileInfo tileinfo = dialog.getSelectTile();
		delete m_image;
		QPixmap img = tileinfo.getTileImage();
		m_image = addPixmap( img.scaled( m_parentView->getSize().width(), m_parentView->getSize().height(), Qt::KeepAspectRatio ) );
		m_parentView->m_selectedTile = tileinfo;
		m_parentView->tileChanged( tileinfo );
	}
}

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