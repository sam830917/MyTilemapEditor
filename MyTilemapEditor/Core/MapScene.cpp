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
	m_layers[layerIndex]->m_tileList[index]->m_tileInfo = tileInfo;
	m_layers[layerIndex]->m_tileList[index]->update();
	m_parentWidget->markCurrentSceneForModified();
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

void MapScene::paintMap( const QMap<int, TileInfo>& tileInfoMap, int layerIndex )
{
	QMap<int, TileInfo>::const_iterator mapIterator = tileInfoMap.constBegin();
	while( mapIterator != tileInfoMap.constEnd() )
	{
		int index = mapIterator.key();
		TileInfo tileInfo = mapIterator.value();
		if( index < 0 )
		{
			continue;
		}
		m_layers[layerIndex]->m_tileList[index]->m_tileInfo = tileInfo;

		++mapIterator;
	}
	update();
}

void MapScene::showTileProperties( const QPointF& mousePos )
{
	// Check out of bound
	QSizeF bound = QSizeF( m_mapInfo.getTileSize().width() * m_mapInfo.getMapSize().width(), m_mapInfo.getTileSize().height() * m_mapInfo.getMapSize().height() );
	if( mousePos.x() >= bound.width() || mousePos.y() >= bound.height() || mousePos.x() <= 0 || mousePos.y() <= 0 )
	{
		return;
	}
	QSize coord = QSize( qFloor( mousePos.x() / m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / m_mapInfo.getTileSize().height() ) );
	if( coord.width() < 0 || coord.height() < 0 )
	{
		return;
	}
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	if( currentIndex == -1 )
	{
		return;
	}
	int index = coord.height() * m_mapInfo.getMapSize().width() + coord.width();
	m_currentSelectedIndex = index;
	Tile* tile = m_layers[currentIndex]->m_tileList[index];

	QMap<QString, QString> informationMap;
	informationMap["X"] = QString("%1").arg( coord.width() );
	informationMap["Y"] = QString("%1").arg( coord.height() );
	if ( tile->m_tileInfo.isValid() )
	{
		informationMap["Tileset"] = tile->m_tileInfo.getTileset()->getFilePath();
		informationMap["Tileset Index"] = QString("%1").arg( tile->m_tileInfo.getIndex() );
	}
	else
	{
		informationMap["Tileset"] = "";
		informationMap["Tileset Index"] = "";
	}
	m_parentWidget->showProperties( informationMap );
}

void MapScene::showSelectedTileProperties()
{
	if ( m_currentSelectedIndex <= -1 )
	{
		return;
	}
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	if( currentIndex == -1 )
	{
		return;
	}
	QSize coord = QSize( m_currentSelectedIndex % m_mapInfo.getMapSize().width(), m_currentSelectedIndex / m_mapInfo.getMapSize().width() );
	Tile* tile = m_layers[currentIndex]->m_tileList[m_currentSelectedIndex];

	QMap<QString, QString> informationMap;
	informationMap["X"] = QString( "%1" ).arg( coord.width() );
	informationMap["Y"] = QString( "%1" ).arg( coord.height() );
	if( tile->m_tileInfo.isValid() )
	{
		informationMap["Tileset"] = tile->m_tileInfo.getTileset()->getFilePath();
		informationMap["Tileset Index"] = QString( "%1" ).arg( tile->m_tileInfo.getIndex() );
	}
	else
	{
		informationMap["Tileset"] = "";
		informationMap["Tileset Index"] = "";
	}
	m_parentWidget->showProperties( informationMap );
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
	m_parentWidget->markCurrentSceneForModified();
}

void MapScene::eraseMap( QSize coord )
{
	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.height() * mapSize.width() + coord.width();
	eraseMap( index );
}

Layer* MapScene::addNewLayer( int zValue )
{
	Layer* newLayer = new Layer( this, zValue );
	m_layers.insert( zValue, newLayer );

	// reorder z value
	for ( int i = 0; i < m_layers.size(); ++i )
	{
		m_layers[i]->setOrder(i);
	}
	return newLayer;
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
		if ( eDrawTool::CURSOR == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() )
			{
				return;
			}
			showTileProperties( mousePos );
			return;
		}

		m_beforeDrawTileInfo.clear();
		int currentIndex = -1;
		m_parentWidget->getLayerIndex( currentIndex );
		if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() )
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
		if( eDrawTool::CURSOR == m_parentWidget->m_drawTool )
		{
			return;
		}

		int currentIndex = -1;
		m_parentWidget->getLayerIndex( currentIndex );
		if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() )
		{
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
		if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() )
		{
			return;
		}

		QUndoCommand* command = new DrawCommand( m_beforeDrawTileInfo, m_layers[currentIndex]->m_tileList );
		m_undoStack->push( command );
		m_parentWidget->disableShortcut( false );
	}
	update();
}