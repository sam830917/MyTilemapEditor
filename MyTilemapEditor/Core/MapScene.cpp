#include "MapScene.h"
#include "UndoCommand.h"
#include "Tileset.h"
#include "Widget/WorkspaceWidget.h"
#include "Utils/ProjectCommon.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QtMath>
#include <QGraphicsView>
#include <QScrollBar>

MapView::MapView( WorkspaceWidget* parent /*= Q_NULLPTR */ )
	:QGraphicsView( parent )
{
}

MapView::MapView( QGraphicsScene* scene, QWidget* parent /*= Q_NULLPTR */ )
	: QGraphicsView( parent )
{
	setScene(scene);
}

void MapView::wheelEvent( QWheelEvent* event )
{
	if( event->modifiers() & Qt::ControlModifier )
	{
		if( event->delta() > 0 )
		{
			QMatrix m = matrix();
			m.scale( 1.25, 1.25 );
			setMatrix( m );
		}
		else
		{
			QMatrix m = matrix();
			m.scale( 0.8, 0.8 );
			setMatrix( m );
		}
	}
	else
	{
		QGraphicsView::wheelEvent( event );
	}
}

MapScene::MapScene( MapInfo mapInfo, WorkspaceWidget* parent /*= Q_NULLPTR*/ )
	:QGraphicsScene( parent ), 
	m_mapInfo(mapInfo),
	m_parentWidget( parent )
{
	m_view = new MapView( this );
	//setBackgroundBrush( QBrush( QColor( 170, 170, 170, 255 ) ) );
	m_undoStack = new QUndoStack( this );

	QSize mapSize = m_mapInfo.getMapSize();
	QSize tileSize = m_mapInfo.getTileSize();
	// Create background canvas
	const int canvasLength = 100;
	QGraphicsRectItem* bgRect = new QGraphicsRectItem();
	bgRect->setPen( QPen( QColor( 0,0,0,0 ) ) );
	QSize canvasSize = QSize( tileSize.width() * mapSize.width() + canvasLength * 2, tileSize.height() * mapSize.height() + canvasLength * 2 );
	bgRect->setRect( -canvasLength, -canvasLength, canvasSize.width(), canvasSize.height() );
	addItem( bgRect );

	QPen linePen( QColor( 170, 170, 170, 255 ) );
	linePen.setWidth( 0 );
	linePen.setStyle( Qt::PenStyle::DotLine );

	int vLineCount = mapSize.width() + 1;
	int hLineCount = mapSize.height() + 1;
	for( int v = 0; v < vLineCount; ++v )
	{
		QGraphicsLineItem* line = new QGraphicsLineItem( QLineF( v * tileSize.width(), 0, v * tileSize.width(), tileSize.height() * mapSize.height() ) );
		line->setPen( linePen );
		line->setZValue(1);
		addItem( line );
	}
	for( int h = 0; h < hLineCount; ++h )
	{
		QGraphicsLineItem* line = new QGraphicsLineItem( QLineF( 0, h * tileSize.height(), tileSize.width() * mapSize.width(), h * tileSize.height() ) );
		line->setPen( linePen );
		line->setZValue(1);
		addItem( line );
	}
}

void MapScene::editMapOnPoint( const QPointF& point )
{
	// Check out of bound
	QSizeF bound = QSizeF( m_mapInfo.getTileSize().width() * m_mapInfo.getMapSize().width(), m_mapInfo.getTileSize().height() * m_mapInfo.getMapSize().height() );
	if ( point.x() >= bound.width() || point.y() >= bound.height() || point.x() <= 0 || point.y() <= 0 )
	{
		return;
	}
	QSize coord = QSize( qFloor( point.x() / m_mapInfo.getTileSize().width() ), qFloor( point.y() / m_mapInfo.getTileSize().height() ) );
	//qDebug() << coord.width() << "," << coord.height();
	if( coord.width() < 0 || coord.height() < 0 )
	{
		return;
	}
	switch( m_parentWidget->getCurrentDrawTool() )
	{
	case eDrawTool::BRUSH:
	{
		paintMap( coord );
		break;
	}
	case eDrawTool::ERASER:
	{
		eraseMap( coord );
		break;
	}
	default:
		break;
	}
}

void MapScene::paintMap( int index )
{
	paintMap( index, getCurrentTile() );
}

void MapScene::paintMap( QSize coord )
{
	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.height() * mapSize.width() + coord.width();
	paintMap( index );
}

void MapScene::paintMap( int index, TileInfo tileInfo, int layerIndex )
{
	if( index < 0 )
	{
		return;
	}
	if ( m_layers[layerIndex]->m_isLock )
	{
		return;
	}
	m_layers[layerIndex]->m_tileList[index]->m_tileInfo = tileInfo;
	m_layers[layerIndex]->m_tileList[index]->update();
	m_parentWidget->modifiedCurrentScene();
}

void MapScene::paintMap( int index, TileInfo tileInfo )
{
	if( index < 0 )
	{
		return;
	}
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	if( currentIndex == -1 )
	{
		return;
	}

	paintMap( index, tileInfo, currentIndex );
}

void MapScene::eraseMap( int index )
{
	if( index < 0 )
	{
		return;
	}
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	if( currentIndex == -1 )
	{
		return;
	}

	m_layers[currentIndex]->m_tileList[index]->m_tileInfo = TileInfo();
	m_layers[currentIndex]->m_tileList[index]->update();
	m_parentWidget->modifiedCurrentScene();
}

void MapScene::eraseMap( QSize coord )
{
	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.height() * mapSize.width() + coord.width();
	eraseMap( index );
}

void MapScene::addNewLayer( int zValue )
{
	Layer* newLayer = new Layer( this, zValue );
	m_layers.push_front( newLayer );

	// reorder z value
	for ( int i = 0; i < m_layers.size(); ++i )
	{
		m_layers[i]->setOrder(i);
	}
}

void MapScene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mousePressEvent( event );
	if( event->button() & Qt::LeftButton )
	{
		if ( eDrawTool::MOVE == m_parentWidget->m_drawTool )
		{
			m_parentWidget->setCursor( Qt::ClosedHandCursor );
			return;
		}
		m_beforeDrawTileInfo.clear();
		int currentIndex = -1;
		m_parentWidget->getLayerIndex( currentIndex );
		if( currentIndex == -1 )
		{
			return;
		}

		for( Tile* tile : m_layers[currentIndex]->m_tileList )
		{
			TileInfo info = tile->getTileInfo();
			m_beforeDrawTileInfo.push_back( info );
		}
		m_parentWidget->disableShortcut( true );
		QPointF mousePos = event->scenePos();
		if( mousePos == QPointF() )
		{
			return;
		}
		editMapOnPoint( mousePos );
	}
}

void MapScene::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mouseMoveEvent( event );

	if( event->buttons() & Qt::LeftButton )
	{
		if( eDrawTool::MOVE == m_parentWidget->m_drawTool )
		{
			QPoint currentPosition = event->screenPos();
			QPoint lastPosition = event->lastScreenPos();
			m_view->verticalScrollBar()->setValue( m_view->verticalScrollBar()->value() - (currentPosition.y() - lastPosition.y()) );
			m_view->horizontalScrollBar()->setValue( m_view->horizontalScrollBar()->value() - (currentPosition.x() - lastPosition.x()) );
			return;
		}
		QPointF mousePos = event->scenePos();
		if( mousePos == QPointF() )
		{
			return;
		}
		editMapOnPoint( mousePos );
	}
}

void MapScene::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mouseReleaseEvent( event );
	if ( eDrawTool::MOVE == m_parentWidget->m_drawTool )
	{
		m_parentWidget->setCursor( Qt::ArrowCursor );
		return;
	}
	if( event->button() & Qt::LeftButton )
	{
		int currentIndex = -1;
		m_parentWidget->getLayerIndex( currentIndex );
		if( currentIndex == -1 )
		{
			return;
		}

		QUndoCommand* command = new DrawCommand( m_beforeDrawTileInfo, m_layers[currentIndex]->m_tileList );
		m_undoStack->push( command );
		m_parentWidget->disableShortcut( false );
	}
	update();
}