#include "MapScene.h"
#include "UndoCommand.h"
#include "Tileset.h"
#include "../Widget/WorkspaceWidget.h"
#include "../Utils/ProjectCommon.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QtMath>

MapScene::MapScene( MapInfo mapInfo, WorkspaceWidget* parent /*= Q_NULLPTR*/ )
	:QGraphicsScene( parent ), 
	m_mapInfo(mapInfo),
	m_parentWidget( parent )
{
	m_undoStack = new QUndoStack( this );

	QPen linePen( Qt::black );
	linePen.setWidth( 3 );
	linePen.setWidth( 2 );
	linePen.setStyle( Qt::PenStyle::DotLine );

	QSize mapSize = m_mapInfo.getMapSize();
	QSize tileSize = m_mapInfo.getTileSize();
	int vLineCount = mapSize.width() + 1;
	int hLineCount = mapSize.height() + 1;
	for( int v = 0; v < vLineCount; ++v )
	{
		addLine( v * tileSize.width(), 0, v * tileSize.width(), tileSize.height() * mapSize.height(), linePen );
	}
	for( int h = 0; h < hLineCount; ++h )
	{
		addLine( 0, h * tileSize.height(), tileSize.width() * mapSize.width(), h * tileSize.height(), linePen );
	}

	// Create Tiles
	for( int y = 0; y < mapSize.height(); ++y )
	{
		for( int x = 0; x < mapSize.width(); ++x )
		{
			Tile* tile = new Tile( this );
			tile->setRect( qreal( x * tileSize.width() ), qreal( y * tileSize.height() ), tileSize.width(), tileSize.height() );
			addItem( tile );
			m_tileList.push_back( tile );
		}
	}
}

void MapScene::editMapOnPoint( const QPointF& point )
{
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
	if ( index < 0 )
	{
		return;
	}
	m_tileList[index]->m_tileInfo = getCurrentTile();
	m_tileList[index]->update();
}

void MapScene::paintMap( QSize coord )
{
	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.height() * mapSize.width() + coord.width();
	paintMap( index );
}

void MapScene::eraseMap( int index )
{
	if( index < 0 )
	{
		return;
	}
	m_tileList[index]->m_tileInfo = TileInfo();
	m_tileList[index]->update();
}

void MapScene::eraseMap( QSize coord )
{
	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.height() * mapSize.width() + coord.width();
	eraseMap( index );
}

void MapScene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mousePressEvent( event );
	if( event->button() & Qt::LeftButton )
	{
		m_beforeDrawTileInfo.clear();
		for( Tile* tile : m_tileList )
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
	QPointF mousePos = event->scenePos();
	if( mousePos == QPointF() )
	{
		return;
	}
	editMapOnPoint( mousePos );
}

void MapScene::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mouseReleaseEvent( event );
	if( event->button() & Qt::LeftButton )
	{
		QUndoCommand* command = new DrawCommand( m_beforeDrawTileInfo, m_tileList );
		m_undoStack->push( command );
		m_parentWidget->disableShortcut( false );
	}
	update();
}

Tile::Tile( MapScene* scene, QGraphicsItem* parent /*= Q_NULLPTR */ )
	:m_mapScene(scene)
{
}

void Tile::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
	if( m_tileInfo.isValid() )
	{
		QRectF rect = boundingRect();
		QSize tileSize = m_tileInfo.getTileset()->getTileSize();
		QSize mapTileSize = m_mapScene->getMapInfo().getTileSize();
		QSize sizeDiff = QSize( mapTileSize.width() - tileSize.width(), mapTileSize.height() - tileSize.height() );
		painter->drawPixmap( rect.x(), rect.y() + sizeDiff.height(), tileSize.width(), tileSize.height(), m_tileInfo.getTileImage() );
	}
}
