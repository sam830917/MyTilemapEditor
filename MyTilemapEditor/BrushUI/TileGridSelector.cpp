#include "BrushUI/TileGridSelector.h"
#include <QGraphicsItem>

class TileGridSelectorScene : public QGraphicsScene
{
	friend class TileGridSelector;

public:
	TileGridSelectorScene( eTileGridType type, const QSize& size );

private:
	TileGridSelector* m_parentView;
	QList<TileGrid*> m_items;
};

class TileGrid : public QGraphicsRectItem
{
	friend class TileGridSelector;
	friend class TileGridSelectorScene;

public:
	TileGrid( const QSize& size );
	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;

protected:
	virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

private:
	QSize m_size;
	bool m_state = true;
};

TileGrid::TileGrid( const QSize& size )
	:QGraphicsRectItem(),
	m_size(size)
{
}

void TileGrid::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= Q_NULLPTR */ )
{
	if ( m_state )
	{
		QPixmap img = QPixmap( ":/MainWindow/Icon/circle.png" );
		QRectF rect = boundingRect();
		QPoint point = QPoint( rect.x() + 0.5f, rect.y() + 0.5f );
		painter->drawPixmap( point.x(), point.y(), m_size.width(), m_size.height(), img );
	}
	else
	{
		QPixmap img = QPixmap( ":/MainWindow/Icon/cross.png" );
		QRectF rect = boundingRect();
		QPoint point = QPoint( rect.x() + 0.5f, rect.y() + 0.5f );
		painter->drawPixmap( point.x(), point.y(), m_size.width(), m_size.height(), img );
	}
}

void TileGrid::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	m_state = !m_state;
	update();
}

TileGridSelectorScene::TileGridSelectorScene( eTileGridType type, const QSize& size )
	:QGraphicsScene()
{
	QPen linePen( QColor( 170, 170, 170, 255 ) );
	linePen.setWidth( 0 );
	linePen.setStyle( Qt::PenStyle::DotLine );

	int vLineCount = 4;
	int hLineCount = 4;
	float wSize = size.width() / 3.f;
	float hSize = size.height() / 3.f;
	for( int v = 0; v < vLineCount; ++v )
	{
		QGraphicsLineItem* line = new QGraphicsLineItem( QLineF( v * wSize, 0, v * wSize, size.height() ) );
		line->setPen( linePen );
		line->setZValue( 1 );
		addItem( line );
	}
	for( int h = 0; h < hLineCount; ++h )
	{
		QGraphicsLineItem* line = new QGraphicsLineItem( QLineF( 0, h * hSize, size.width(), h * hSize ) );
		line->setPen( linePen );
		line->setZValue( 1 );
		addItem( line );
	}

	switch( type )
	{
	case eTileGridType::EDGE:
	{
		for( int y = 0; y < 3; ++y )
		{
			for( int x = 0; x < 3; ++x )
			{
				if( x == 1 && y == 1 )
				{
					continue;
				}
				if( x == 1 || y == 1 )
				{
					TileGrid* item = new TileGrid( QSize( wSize, hSize ) );
					item->setRect( x * wSize, y * hSize, wSize, hSize );
					addItem( item );
					m_items.push_back( item );
				}
			}
		}
		break;
	}
	case eTileGridType::CORNER:
	{
		for( int y = 0; y < 3; ++y )
		{
			for( int x = 0; x < 3; ++x )
			{
				if( x == 1 || y == 1 )
				{
					continue;
				}
				TileGrid* item = new TileGrid( QSize( wSize, hSize ) );
				item->setRect( x * wSize, y * hSize, wSize, hSize );
				addItem( item );
				m_items.push_back( item );
			}
		}
		break;
	}
	case eTileGridType::EDGE_AND_CORNER:
	{
		for( int y = 0; y < 3; ++y )
		{
			for( int x = 0; x < 3; ++x )
			{
				if ( x == 1 && y == 1 )
				{
					continue;
				}
				TileGrid* item = new TileGrid( QSize( wSize, hSize ) );
				item->setRect( x * wSize, y * hSize, wSize, hSize );
				addItem( item );
				m_items.push_back( item );
			}
		}
		break;
	}
	default:
		break;
	}
}

//----------------------------------------------------------------------------------------------------
TileGridSelector::TileGridSelector( eTileGridType type, const QSize& size )
	:QGraphicsView(),
	m_size( size ),
	m_type( type )
{
	setMaximumSize( size + QSize(4,4) );
	setMinimumSize( size + QSize(4,4) );
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

	m_scene = new TileGridSelectorScene( type, size );
	setScene( m_scene );
	m_scene->m_parentView = this;
}

TileGridSelector::~TileGridSelector()
{
}

QList<bool> TileGridSelector::getGridState() const
{
	QList<bool> states;
	for ( int i = 0; i < m_scene->m_items.size(); ++i )
	{
		states.push_back( m_scene->m_items[i]->m_state );
	}
	return states;
}

void TileGridSelector::setGridState( QList<bool> states ) const
{
	for( int i = 0; i < states.size(); ++i )
	{
		m_scene->m_items[i]->m_state = states[i];
		m_scene->m_items[i]->update();
	}
}
