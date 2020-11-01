#include "MapScene.h"
#include "UndoCommand.h"
#include "Tileset.h"
#include "Widget/WorkspaceWidget.h"
#include "Utils/ProjectCommon.h"
#include "Brush/BrushCommon.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QtMath>
#include <QGraphicsView>
#include <QScrollBar>
#include <qDebug>

class SelectMask : public QGraphicsRectItem
{
	friend class MapScene;

public:
	SelectMask( MapScene* scene, QPoint coord );
	~SelectMask();

	void setSelected( bool isSelected );
private:
	virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR ) override;

private:
	MapScene* m_scene;
	QPoint m_coord;
	bool m_isSelected = false;
};

SelectMask::SelectMask( MapScene* scene, QPoint coord )
	:QGraphicsRectItem(NULL),
	m_scene(scene),
	m_coord(coord)
{
	setZValue(1);
}

SelectMask::~SelectMask()
{
}

void SelectMask::setSelected( bool isSelected )
{
	m_isSelected = isSelected;
}

void SelectMask::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= Q_NULLPTR */ )
{
	if( !m_scene->m_showSelection )
		return;

	if ( m_isSelected )
	{
		QRectF rect = boundingRect();
		QSize& tileSize = m_scene->getMapInfo().getTileSize();
		painter->setBrush( QBrush( QColor( 0, 0, 100, 100 ) ) );
		painter->setPen( QPen( QColor( 0, 0, 0, 0 ) ) );
		painter->drawRect( (rect.x() + 0.5f), (rect.y() + 0.5f), tileSize.width(), tileSize.height() );
	}
}

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

	m_selectedTileItemList.reserve( mapSize.height() * mapSize.width() );
	// Create masks
	for( int y = 0; y < mapSize.height(); ++y )
	{
		for( int x = 0; x < mapSize.width(); ++x )
		{
			SelectMask* mask = new SelectMask( this, QPoint( x, y ) );
			mask->setRect( qreal( x * tileSize.width() ), qreal( y * tileSize.height() ), tileSize.width(), tileSize.height() );
			addItem( mask );
			m_selectedTileItemList.push_back( mask );
		}
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
		QList<TileModified> modifiedList;
		m_parentWidget->getPaintMapModified( modifiedList, QPoint( coord.width(), coord.height() ), m_parentWidget->getCurrentDrawTool() );
		for( TileModified m : modifiedList )
		{
			paintMap( m.m_coordinate, m.m_tileInfo );
		}
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

void MapScene::editMapByFloodFill( int layerIndex, const QPoint& coord )
{
	TileInfo currentTileInfo = m_layers[layerIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo;
	paintTileByFloodFill( layerIndex, coord, currentTileInfo, getCurrentTile() );
}

void MapScene::selectTilesByFloodFill( int layerIndex, const QPoint& coord )
{
	for( SelectMask* mask : m_selectedTileItemList )
	{
		mask->setSelected( false );
	}
	TileInfo currentTileInfo = m_layers[layerIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo;
	selectTilesByFloodFill( layerIndex, coord, currentTileInfo, getCurrentTile() );
}

int MapScene::getCurrentLayerIndex()
{
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	return currentIndex;
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

void MapScene::paintMap( QPoint coord, TileInfo tileInfo )
{
	if ( coord.x() >= m_mapInfo.getMapSize().width() || coord.y() >= m_mapInfo.getMapSize().height() )
	{
		return;
	}
	int index = m_mapInfo.getIndex( coord );
	paintMap( index, tileInfo );
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
		informationMap["Tileset"] = tile->m_tileInfo.getTileset()->getName();
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
	if ( m_isSelectedMoreThanOneTile )
	{
		QMap<QString, QString> informationMap;
		informationMap["X"] = QString( "..." );
		informationMap["Y"] = QString( "..." );
		informationMap["Tileset"] = "...";
		informationMap["Tileset Index"] = "...";
		m_parentWidget->showProperties( informationMap );
		return;
	}
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
		informationMap["Tileset"] = tile->m_tileInfo.getTileset()->getName();
		informationMap["Tileset Index"] = QString( "%1" ).arg( tile->m_tileInfo.getIndex() );
	}
	else
	{
		informationMap["Tileset"] = "";
		informationMap["Tileset Index"] = "";
	}
	m_parentWidget->showProperties( informationMap );
}

void MapScene::paintTileByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, const TileInfo& newTileInfo )
{
	if( coord.x() < 0 || coord.x() >= m_mapInfo.getMapSize().width() || coord.y() < 0 || coord.y() >= m_mapInfo.getMapSize().height() )
		return;

	if ( m_layers[layerIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo != currentTileInfo )
		return;

	if( m_layers[layerIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo == newTileInfo )
		return;

	paintMap( m_mapInfo.getIndex(coord), newTileInfo, layerIndex );

	paintTileByFloodFill( layerIndex, QPoint( coord.x() + 1, coord.y() ), currentTileInfo, newTileInfo );
	paintTileByFloodFill( layerIndex, QPoint( coord.x() - 1, coord.y() ), currentTileInfo, newTileInfo );
	paintTileByFloodFill( layerIndex, QPoint( coord.x(), coord.y() + 1 ), currentTileInfo, newTileInfo );
	paintTileByFloodFill( layerIndex, QPoint( coord.x(), coord.y() - 1 ), currentTileInfo, newTileInfo );
}

void MapScene::selectTilesByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, const TileInfo& newTileInfo )
{
	if( coord.x() < 0 || coord.x() >= m_mapInfo.getMapSize().width() || coord.y() < 0 || coord.y() >= m_mapInfo.getMapSize().height() )
		return;

	if( m_layers[layerIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo != currentTileInfo )
		return;

	if( m_layers[layerIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo == newTileInfo )
		return;

	if ( m_selectedTileItemList[m_mapInfo.getIndex( coord )]->m_isSelected )
		return;

	m_selectedTileItemList[m_mapInfo.getIndex( coord )]->setSelected( true );

	selectTilesByFloodFill( layerIndex, QPoint( coord.x() + 1, coord.y() ), currentTileInfo, newTileInfo );
	selectTilesByFloodFill( layerIndex, QPoint( coord.x() - 1, coord.y() ), currentTileInfo, newTileInfo );
	selectTilesByFloodFill( layerIndex, QPoint( coord.x(), coord.y() + 1 ), currentTileInfo, newTileInfo );
	selectTilesByFloodFill( layerIndex, QPoint( coord.x(), coord.y() - 1 ), currentTileInfo, newTileInfo );
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

void MapScene::setIsShowSelection( bool isShow )
{
	m_showSelection = isShow;
	updateSelection();
}

void MapScene::updateSelection()
{
	m_isSelectedMoreThanOneTile = false;
	bool hasSelected = false;
	for( SelectMask* mask : m_selectedTileItemList )
	{
		if ( mask->m_isSelected )
		{
			if ( hasSelected )
			{
				m_isSelectedMoreThanOneTile = true;
			}
			hasSelected = true;
		}
		mask->update();
	}
}

void MapScene::eraseSelectedTiles()
{
	if( !m_showSelection )
	{
		return;
	}
	m_beforeDrawTileInfo.clear();
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() )
	{
		return;
	}
	m_beforeDrawTileInfo.reserve( m_layers[currentIndex]->m_tileList.size() );
	for( Tile* tile : m_layers[currentIndex]->m_tileList )
	{
		TileInfo& info = tile->getTileInfo();
		m_beforeDrawTileInfo.push_back( info );
	}
	for( int i = 0; i < m_selectedTileItemList.size(); ++i )
	{
		if( m_selectedTileItemList[i]->m_isSelected )
		{
			eraseMap( i );
		}
	}
	QUndoCommand* command = new DrawCommand( m_beforeDrawTileInfo, m_layers[currentIndex]->m_tileList );
	m_undoStack->push( command );
	setIsShowSelection( false );
}

void MapScene::selecteAllTiles()
{
	if( m_parentWidget->m_drawTool != eDrawTool::CURSOR )
	{
		return;
	}
	setIsShowSelection( true );
	for( SelectMask* mask : m_selectedTileItemList )
	{
		mask->setSelected( true );
	}
	updateSelection();
}

void MapScene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mousePressEvent( event );
	setIsShowSelection( false );
	if( event->button() & Qt::LeftButton )
	{
		if ( eDrawTool::MOVE == m_parentWidget->m_drawTool )
		{
			m_parentWidget->setCursor( Qt::ClosedHandCursor );
			return;
		}
		else if ( eDrawTool::CURSOR == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() )
			{
				return;
			}
			m_isSelectedMoreThanOneTile = false;
			m_startPos = mousePos;
			QPoint coord = QPoint( qFloor( mousePos.x() / m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / m_mapInfo.getTileSize().height() ) );
			m_selectedMinCoord = coord;
			m_selectedMaxCoord = coord + QPoint( 1, 1 );
			for ( SelectMask* mask : m_selectedTileItemList )
			{
				mask->setSelected( false );
				if( mask->m_coord.x() >= m_selectedMinCoord.x() && mask->m_coord.x() < m_selectedMaxCoord.x() && 
					mask->m_coord.y() < m_selectedMaxCoord.y() && mask->m_coord.y() >= m_selectedMinCoord.y() )
				{
					mask->setSelected( true );
				}
			}
			setIsShowSelection( true );
			showTileProperties( mousePos );
			return;
		}
		else if ( eDrawTool::BUCKET == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() )
			{
				return;
			}
			QSizeF bound = QSizeF( m_mapInfo.getTileSize().width() * m_mapInfo.getMapSize().width(), m_mapInfo.getTileSize().height() * m_mapInfo.getMapSize().height() );
			if( mousePos.x() >= bound.width() || mousePos.y() >= bound.height() || mousePos.x() <= 0 || mousePos.y() <= 0 )
			{
				return;
			}
			QPoint coord = QPoint( qFloor( mousePos.x() / m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / m_mapInfo.getTileSize().height() ) );

			// Flood fill Algorithm
			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 )
			{
				return;
			}
			m_beforeDrawTileInfo.clear();
			m_beforeDrawTileInfo.reserve( m_layers[currentIndex]->m_tileList.size() );
			for( Tile* tile : m_layers[currentIndex]->m_tileList )
			{
				TileInfo& info = tile->getTileInfo();
				m_beforeDrawTileInfo.push_back( info );
			}
			editMapByFloodFill( currentIndex, coord );
			QUndoCommand* command = new DrawCommand( m_beforeDrawTileInfo, m_layers[currentIndex]->m_tileList );
			m_undoStack->push( command );
		}
		else if ( eDrawTool::MAGIC_WAND == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() )
			{
				return;
			}
			QSizeF bound = QSizeF( m_mapInfo.getTileSize().width() * m_mapInfo.getMapSize().width(), m_mapInfo.getTileSize().height() * m_mapInfo.getMapSize().height() );
			if( mousePos.x() >= bound.width() || mousePos.y() >= bound.height() || mousePos.x() <= 0 || mousePos.y() <= 0 )
			{
				return;
			}
			QPoint coord = QPoint( qFloor( mousePos.x() / m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / m_mapInfo.getTileSize().height() ) );

			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 )
			{
				return;
			}
			selectTilesByFloodFill( currentIndex, coord );
			setIsShowSelection( true );
			showSelectedTileProperties();
		}
		else if ( eDrawTool::SELECT_SAME_TILE == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() )
			{
				return;
			}
			QSizeF bound = QSizeF( m_mapInfo.getTileSize().width() * m_mapInfo.getMapSize().width(), m_mapInfo.getTileSize().height() * m_mapInfo.getMapSize().height() );
			if( mousePos.x() >= bound.width() || mousePos.y() >= bound.height() || mousePos.x() <= 0 || mousePos.y() <= 0 )
			{
				return;
			}
			QPoint coord = QPoint( qFloor( mousePos.x() / m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / m_mapInfo.getTileSize().height() ) );

			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 )
			{
				return;
			}
			TileInfo currentTileInfo = m_layers[currentIndex]->m_tileList[m_mapInfo.getIndex( coord )]->m_tileInfo;
			for( int i = 0; i < m_selectedTileItemList.size(); ++i )
			{
				if ( m_layers[currentIndex]->m_tileList[i]->m_tileInfo == currentTileInfo )
				{
					m_selectedTileItemList[i]->setSelected( true );
				}
				else
				{
					m_selectedTileItemList[i]->setSelected( false );
				}
			}

			setIsShowSelection( true );
			showSelectedTileProperties();
		}
		// Draw or erase mode
		else if ( eDrawTool::BRUSH == m_parentWidget->m_drawTool ||
				eDrawTool::ERASER == m_parentWidget->m_drawTool )
		{
			m_beforeDrawTileInfo.clear();
			int currentIndex = -1;
			m_parentWidget->getLayerIndex( currentIndex );
			if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() )
			{
				return;
			}

			m_beforeDrawTileInfo.reserve( m_layers[currentIndex]->m_tileList.size() );
			for( Tile* tile : m_layers[currentIndex]->m_tileList )
			{
				TileInfo& info = tile->getTileInfo();
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
	update();
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
		else if( eDrawTool::CURSOR == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() || m_selectedTileItemList.isEmpty() )
			{
				return;
			}
			QSize& tileSize = m_mapInfo.getTileSize();
			QPointF minPoint = QPointF( qMin( m_startPos.x(), mousePos.x() ), qMin( m_startPos.y(), mousePos.y() ) );
			QPointF maxPoint = QPointF( qMax( m_startPos.x(), mousePos.x() ), qMax( m_startPos.y(), mousePos.y() ) );

			m_selectedMinCoord = QPoint( qFloor( minPoint.x() / tileSize.width() ), qFloor( minPoint.y() / tileSize.height() ) );
			m_selectedMaxCoord = QPoint( qCeil( maxPoint.x() / tileSize.width() ), qCeil( maxPoint.y() / tileSize.height() ) );

			QPoint coordDiff = m_selectedMaxCoord - m_selectedMinCoord;
			if( coordDiff.x() > 1 || coordDiff.y() > 1 )
			{
				m_isSelectedMoreThanOneTile = true;
			}
			else
			{
				m_isSelectedMoreThanOneTile = false;
			}
			for( SelectMask* mask : m_selectedTileItemList )
			{
				mask->setSelected( false );
				if( mask->m_coord.x() >= m_selectedMinCoord.x() && mask->m_coord.x() < m_selectedMaxCoord.x() &&
					mask->m_coord.y() < m_selectedMaxCoord.y() && mask->m_coord.y() >= m_selectedMinCoord.y() )
				{
					mask->setSelected( true );
				}
			}
			setIsShowSelection( true );
			showSelectedTileProperties();
			
			return;
		}
		// Draw or erase mode
		else if( eDrawTool::BRUSH == m_parentWidget->m_drawTool ||
			eDrawTool::ERASER == m_parentWidget->m_drawTool )
		{
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
}

void MapScene::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mouseReleaseEvent( event );

	if( event->button() & Qt::LeftButton )
	{
		if( eDrawTool::MOVE == m_parentWidget->m_drawTool )
		{
			m_parentWidget->setCursor( Qt::ArrowCursor );
			return;
		}
		else if( eDrawTool::CURSOR == m_parentWidget->m_drawTool )
		{
			return;
		}
		// Draw or erase mode
		else if( eDrawTool::BRUSH == m_parentWidget->m_drawTool ||
			eDrawTool::ERASER == m_parentWidget->m_drawTool )
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
	}
	update();
}