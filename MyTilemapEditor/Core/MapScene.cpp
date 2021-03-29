#include "MapScene.h"
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
#include <QWheelEvent>

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

MapView::MapView( MapScene* scene, QWidget* parent /*= Q_NULLPTR */ )
	: QGraphicsView( parent ),
	m_mapScene(scene)
{
	setScene(scene);
}

void MapView::updateCursor()
{
	viewport()->setCursor( m_mapScene->m_parentWidget->m_currentCursor );
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

void MapView::enterEvent( QEvent* event )
{
	QGraphicsView::enterEvent(event);
	viewport()->setCursor( m_mapScene->m_parentWidget->m_currentCursor );
}

MapScene::MapScene( MapInfo mapInfo, WorkspaceWidget* parent )
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

	m_undoStack->setUndoLimit( 30 );
}

MapScene::MapScene( MapInfo mapInfo )
	:QGraphicsScene(),
	m_mapInfo( mapInfo )
{
	m_view = new MapView( this );
}

MapScene::~MapScene()
{
	delete m_view;
	for ( int i = 0; i < m_layers.size(); ++i )
	{
		delete m_layers[i];
		m_layers[i] = Q_NULLPTR;
	}
	for ( int i = 0; i < m_selectedTileItemList.size(); ++i )
	{
		delete m_selectedTileItemList[i];
		m_selectedTileItemList[i] = Q_NULLPTR;
	}
	if ( m_undoStack )
	{
		delete m_undoStack;
		m_undoStack = Q_NULLPTR;
	}
}

TileInfo MapScene::getTileInfo( int tileIndex, int layerIndex ) const
{
	if ( m_layers[layerIndex]->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
	{
		TileLayer* layer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
		return layer->getTileInfo(tileIndex);
	}
	return TileInfo();
}

bool MapScene::getIsMarked( int tileIndex, int layerIndex ) const
{
	if( m_layers[layerIndex]->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		MarkerLayer* layer = dynamic_cast<MarkerLayer*>(m_layers[layerIndex]);
		return layer->m_tileList[tileIndex]->isMarked();
	}
	return false;
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
	if ( m_lastPaintCoord == QPoint( coord.width(), coord.height() ) )
	{
		return;
	}
	else
	{
		m_lastPaintCoord = QPoint( coord.width(), coord.height() );
	}
	switch( m_parentWidget->getCurrentDrawTool() )
	{
	case eDrawTool::BRUSH:
	{
		bool isDefault = true;
		m_parentWidget->isDefalutBrush( isDefault );
		int layerIndex = getCurrentLayerIndex();
		Layer* layer = m_layers[layerIndex];
		if ( isDefault || layer->getLayerInfo().getLayerType() != eLayerType::TILE_LAYER )
		{
			QList<TileInfo> selectedTileList = getCurrentTiles();
			QSize regionSize = getSelectedTilesRegionSize();
			QSize mapSize = m_mapInfo.getMapSize();
			if ( selectedTileList.empty() )
			{
				return;
			}
			if ( layer->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
			{
				for( int h = 0; h < regionSize.height(); ++h )
				{
					for( int w = 0; w < regionSize.width(); ++w )
					{
						TileInfo t = selectedTileList[h * regionSize.width() + w];
						QPoint pCoord = QPoint( coord.width() + w, coord.height() + h );

						TileModified tileModified( pCoord, getTileInfo( m_mapInfo.getIndex( pCoord ), layerIndex ) );
						if( !m_oldTileModifiedList.contains( tileModified ) )
						{
							m_oldTileModifiedList.insert( tileModified );
						}
						paintMap( pCoord, t );
					}
				}
			}
			else if ( layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
			{
				for( int h = 0; h < regionSize.height(); ++h )
				{
					for( int w = 0; w < regionSize.width(); ++w )
					{
						QPoint pCoord = QPoint( coord.width() + w, coord.height() + h );

						TileMarkerModified tileModified( pCoord, getIsMarked( m_mapInfo.getIndex( pCoord ), layerIndex ) );
						if( !m_oldTileMarkerModifiedList.contains( tileModified ) )
						{
							m_oldTileMarkerModifiedList.insert( tileModified );
						}
						paintMap( pCoord, TileInfo() );
					}
				}
			}
		}
		else
		{
			QList<TileModified> modifiedList;
			m_parentWidget->getPaintMapModified( modifiedList, QPoint( coord.width(), coord.height() ), m_parentWidget->getCurrentDrawTool() );
			for( TileModified m : modifiedList )
			{
				TileModified tileModified( m.m_coordinate, getTileInfo( m_mapInfo.getIndex( m.m_coordinate ), layerIndex ) );
				if( !m_oldTileModifiedList.contains( tileModified ) )
				{
					m_oldTileModifiedList.insert( tileModified );
				}
				paintMap( m.m_coordinate, m.m_tileInfo );
			}
		}
		break;
	}
	case eDrawTool::ERASER:
	{
		bool isDefault = true;
		m_parentWidget->isDefalutBrush( isDefault );
		int layerIndex = getCurrentLayerIndex();
		Layer* layer = m_layers[layerIndex];
		if( isDefault || layer->getLayerInfo().getLayerType() != eLayerType::TILE_LAYER )
		{
			QPoint pCoord = QPoint( coord.width(), coord.height() );
			TileModified tileModified( pCoord, getTileInfo( m_mapInfo.getIndex( pCoord ), layerIndex ) );
			if( !m_oldTileModifiedList.contains( tileModified ) )
			{
				m_oldTileModifiedList.insert( tileModified );
			}
			eraseMap( pCoord );
		}
		else
		{
			QList<TileModified> modifiedList;
			m_parentWidget->getPaintMapModified( modifiedList, QPoint( coord.width(), coord.height() ), m_parentWidget->getCurrentDrawTool() );
			for( TileModified m : modifiedList )
			{
				if( !m_oldTileModifiedList.contains( m ) )
				{
					m_oldTileModifiedList.insert( m );
				}
				paintMap( m.m_coordinate, m.m_tileInfo );
			}
		}
		break;
	}
	default:
		break;
	}
}

QList<QPoint> MapScene::editMapByFloodFill( int layerIndex, const QPoint& coord )
{
	QList<QPoint> readyToPaintTileIndexes;
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
	TileInfo currentTileInfo = tileLayer->getTileInfo( coord.x(), coord.y() );
	paintTileByFloodFill( layerIndex, coord, currentTileInfo, readyToPaintTileIndexes );

	return readyToPaintTileIndexes;
}

void MapScene::selectTilesByFloodFill( int layerIndex, const QPoint& coord )
{
	for( SelectMask* mask : m_selectedTileItemList )
	{
		mask->setSelected( false );
	}
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
	TileInfo currentTileInfo = tileLayer->getTileInfo( coord.x(), coord.y() );
	selectTilesByFloodFill( layerIndex, coord, currentTileInfo, getCurrentTile() );
}

int MapScene::getCurrentLayerIndex() const
{
	int currentIndex = -1;
	m_parentWidget->getLayerIndex( currentIndex );
	return currentIndex;
}

void MapScene::paintMap( int index )
{
	QList<TileInfo> selectedTileList = getCurrentTiles();
	QSize regionSize = getSelectedTilesRegionSize();
	QSize mapSize = m_mapInfo.getMapSize();
	for ( int h = 0; h < regionSize.height(); ++h )
	{
		for( int w = 0; w < regionSize.width(); ++w )
		{
			int displacementIndex = h * mapSize.height() + w;
			TileInfo t = selectedTileList[h * regionSize.width() + w];

			paintMap( index + displacementIndex, t );
		}
	}
}

void MapScene::paintMap( QSize coord )
{
	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.height() * mapSize.width() + coord.width();
	paintMap( index );
}

void MapScene::paintMap( int index, TileInfo tileInfo, int layerIndex )
{
	if( m_layers[layerIndex]->getLayerInfo().isLock() )
	{
		return;
	}
	Layer* layer = m_layers[layerIndex];
	switch( layer->getLayerInfo().getLayerType() )
	{
	case eLayerType::TILE_LAYER:
	{
		TileLayer* tileLayer = dynamic_cast<TileLayer*>(layer);
		tileLayer->setTileInfo( index, tileInfo );
		break;
	}
	case eLayerType::MARKER_LAYER:
	{
		MarkerLayer* tileLayer = dynamic_cast<MarkerLayer*>(layer);
		tileLayer->markTile( index, true );
		break;
	}
	default:
		break;
	}
	m_parentWidget->markCurrentSceneForModified();
}

void MapScene::paintMap( const QPoint& coord, TileInfo tileInfo, int layerIndex )
{
	int index = m_mapInfo.getIndex(coord);
	paintMap( index, tileInfo, layerIndex );
}

void MapScene::paintMap( int index, TileInfo tileInfo )
{
	if( index < 0 )
	{
		return;
	}
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1  || m_layers[currentIndex]->getLayerInfo().isLock() )
	{
		return;
	}

	paintMap( index, tileInfo, currentIndex );
}

void MapScene::paintMap( const QMap<int, TileInfo>& tileInfoMap, int layerIndex )
{
	if( m_layers[layerIndex]->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
	{
		QMap<int, TileInfo>::const_iterator mapIterator = tileInfoMap.constBegin();
		TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
		while( mapIterator != tileInfoMap.constEnd() )
		{
			int index = mapIterator.key();
			TileInfo tileInfo = mapIterator.value();
			if( index < 0 )
			{
				continue;
			}
			tileLayer->setTileInfo( index, tileInfo );

			++mapIterator;
		}
	}
	else if ( m_layers[layerIndex]->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		QMap<int, TileInfo>::const_iterator mapIterator = tileInfoMap.constBegin();
		MarkerLayer* tileLayer = dynamic_cast<MarkerLayer*>(m_layers[layerIndex]);
		while( mapIterator != tileInfoMap.constEnd() )
		{
			int index = mapIterator.key();
			if( index < 0 )
			{
				continue;
			}
			tileLayer->markTile( index, true );

			++mapIterator;
		}
	}
	update();
}

void MapScene::paintMap( QPoint coord, TileInfo tileInfo )
{
	if ( coord.x() < 0 || coord.y() < 0 || coord.x() >= m_mapInfo.getMapSize().width() || coord.y() >= m_mapInfo.getMapSize().height() )
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
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1 )
	{
		return;
	}
	int index = coord.height() * m_mapInfo.getMapSize().width() + coord.width();
	m_currentSelectedIndex = index;

	QMap<QString, QString> informationMap;
	informationMap["X"] = QString( "%1" ).arg( coord.width() );
	informationMap["Y"] = QString( "%1" ).arg( coord.height() );

	Layer* layer = m_layers[currentIndex];
	if ( layer->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
	{
		TileLayer* tileLayer = dynamic_cast<TileLayer*>(layer);
		TileInfo tileInfo = tileLayer->getTileInfo( index );
		if( tileInfo.isValid() )
		{
			informationMap["Tileset"] = tileInfo.getTileset()->getName();
			informationMap["Tileset Index"] = QString( "%1" ).arg( tileInfo.getIndex() );
		}
		else
		{
			informationMap["Tileset"] = "";
			informationMap["Tileset Index"] = "";
		}
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
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1 )
	{
		return;
	}
	QSize coord = QSize( m_currentSelectedIndex % m_mapInfo.getMapSize().width(), m_currentSelectedIndex / m_mapInfo.getMapSize().width() );
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
	TileInfo tileInfo = tileLayer->getTileInfo( m_currentSelectedIndex );

	QMap<QString, QString> informationMap;
	informationMap["X"] = QString( "%1" ).arg( coord.width() );
	informationMap["Y"] = QString( "%1" ).arg( coord.height() );
	if( tileInfo.isValid() )
	{
		informationMap["Tileset"] = tileInfo.getTileset()->getName();
		informationMap["Tileset Index"] = QString( "%1" ).arg( tileInfo.getIndex() );
	}
	else
	{
		informationMap["Tileset"] = "";
		informationMap["Tileset Index"] = "";
	}
	m_parentWidget->showProperties( informationMap );
}

void MapScene::paintTileByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, QList<QPoint>& readyToPaintTileIndexes )
{
	if( coord.x() < 0 || coord.x() >= m_mapInfo.getMapSize().width() || coord.y() < 0 || coord.y() >= m_mapInfo.getMapSize().height() )
		return;

	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
	TileInfo tileInfo = tileLayer->getTileInfo( coord.x(), coord.y() );
	if ( tileInfo != currentTileInfo )
		return;

	for ( int i = 0; i < readyToPaintTileIndexes.size(); ++i )
	{
		if ( readyToPaintTileIndexes[i] == coord )
		{
			return;
		}
	}
	readyToPaintTileIndexes.push_back( coord );

	paintTileByFloodFill( layerIndex, QPoint( coord.x() + 1, coord.y() ), currentTileInfo, readyToPaintTileIndexes );
	paintTileByFloodFill( layerIndex, QPoint( coord.x() - 1, coord.y() ), currentTileInfo, readyToPaintTileIndexes );
	paintTileByFloodFill( layerIndex, QPoint( coord.x(), coord.y() + 1 ), currentTileInfo, readyToPaintTileIndexes );
	paintTileByFloodFill( layerIndex, QPoint( coord.x(), coord.y() - 1 ), currentTileInfo, readyToPaintTileIndexes );
}

void MapScene::selectTilesByFloodFill( int layerIndex, const QPoint& coord, const TileInfo& currentTileInfo, const TileInfo& newTileInfo )
{
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
	if( coord.x() < 0 || coord.x() >= m_mapInfo.getMapSize().width() || coord.y() < 0 || coord.y() >= m_mapInfo.getMapSize().height() )
		return;

	if( tileLayer->getTileInfo( coord.x(), coord.y() ) != currentTileInfo )
		return;

	if( tileLayer->getTileInfo( coord.x(), coord.y() ) == newTileInfo )
		return;

	if ( m_selectedTileItemList[m_mapInfo.getIndex( coord )]->m_isSelected )
		return;

	m_selectedTileItemList[m_mapInfo.getIndex( coord )]->setSelected( true );

	selectTilesByFloodFill( layerIndex, QPoint( coord.x() + 1, coord.y() ), currentTileInfo, newTileInfo );
	selectTilesByFloodFill( layerIndex, QPoint( coord.x() - 1, coord.y() ), currentTileInfo, newTileInfo );
	selectTilesByFloodFill( layerIndex, QPoint( coord.x(), coord.y() + 1 ), currentTileInfo, newTileInfo );
	selectTilesByFloodFill( layerIndex, QPoint( coord.x(), coord.y() - 1 ), currentTileInfo, newTileInfo );
}

void MapScene::eraseMap( int tileIndex, int layerIndex )
{
	if( tileIndex < 0 )
	{
		return;
	}
	Layer* layer = m_layers[layerIndex];
	switch( layer->getLayerInfo().getLayerType() )
	{
	case eLayerType::TILE_LAYER:
	{
		TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[layerIndex]);
		tileLayer->setTileInfo( tileIndex, TileInfo() );
		break;
	}
	case eLayerType::MARKER_LAYER:
	{
		MarkerLayer* tileLayer = dynamic_cast<MarkerLayer*>(layer);
		tileLayer->markTile( tileIndex, false );
		break;
	}
	default:
		break;
	}

	m_parentWidget->markCurrentSceneForModified();
}

void MapScene::eraseMap( int index )
{
	if( index < 0 )
	{
		return;
	}
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1 )
	{
		return;
	}
	eraseMap( index, currentIndex );
}

void MapScene::eraseMap( QPoint coord )
{
	if( coord.x() < 0 || coord.y() < 0 || coord.x() >= m_mapInfo.getMapSize().width() || coord.y() >= m_mapInfo.getMapSize().height() )
	{
		return;
	}

	QSize mapSize = m_mapInfo.getMapSize();
	int index = coord.y() * mapSize.width() + coord.x();
	eraseMap( index );
}

void MapScene::eraseMap( const QPoint& coord, int layerIndex )
{
	int index = coord.y() * m_mapInfo.getMapSize().width() + coord.x();
	eraseMap( index, layerIndex );
}

TileLayer* MapScene::addNewLayer( int zValue )
{
	TileLayer* newLayer = new TileLayer( this, zValue );
	m_layers.insert( zValue, newLayer );

	// reorder z value
	for ( int i = 0; i < m_layers.size(); ++i )
	{
		m_layers[i]->setOrder(i);
	}
	return newLayer;
}

MarkerLayer* MapScene::addNewMarkerLayer( int zValue )
{
	MarkerLayer* newLayer = new MarkerLayer( this, zValue );
	m_layers.insert( zValue, newLayer );

	// reorder z value
	for( int i = 0; i < m_layers.size(); ++i )
	{
		m_layers[i]->setOrder( i );
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
	m_oldTileModifiedList.clear();
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() || !m_layers[currentIndex]->getLayerInfo().isVisible() )
	{
		return;
	}
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);

	int layerIndex = getCurrentLayerIndex();
	for( int i = 0; i < m_selectedTileItemList.size(); ++i )
	{
		if( m_selectedTileItemList[i]->m_isSelected )
		{
			m_oldTileModifiedList.insert( TileModified( m_mapInfo.getCoord(i), getTileInfo( i, layerIndex ) ) );
			eraseMap( i );
		}
	}
	QUndoCommand* command = new DrawCommand( this, layerIndex, m_oldTileModifiedList );
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

QList<TileModified> MapScene::getCopiedTiles() const
{
	QList<TileModified> tileModifiedList;
	if( !m_showSelection )
	{
		return tileModifiedList;
	}
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1 || !m_layers[currentIndex]->getLayerInfo().isVisible() )
	{
		return tileModifiedList;
	}

	QPoint smallestPoint = QPoint( m_mapInfo.getMapSize().width(), m_mapInfo.getMapSize().height() );
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
	for( int i = 0; i < m_selectedTileItemList.size(); ++i )
	{
		if( m_selectedTileItemList[i]->m_isSelected )
		{
			QPoint coord = m_mapInfo.getCoord( i );
			TileInfo tileinfo = tileLayer->getTileInfo(i);
			TileModified tileModified( coord, tileinfo );
			tileModifiedList.push_back( tileModified );
			smallestPoint.setX( qMin( smallestPoint.x(), coord.x() ) );
			smallestPoint.setY( qMin( smallestPoint.y(), coord.y() ) );
		}
	}
	for( int i = 0; i < tileModifiedList.size(); ++i )
	{
		tileModifiedList[i].m_coordinate -= smallestPoint;
	}

	return tileModifiedList;
}

void MapScene::pasteTilesOnCoord( const QPoint& coord, const QList<TileModified>& copiedTileList )
{
	if ( copiedTileList.empty() )
	{
		return;
	}
	int currentIndex = getCurrentLayerIndex();
	if( currentIndex == -1 )
	{
		return;
	}
	m_oldTileModifiedList.clear();
	TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
	int layerIndex = getCurrentLayerIndex();

	for( TileModified tileModified : copiedTileList )
	{
		QPoint point = tileModified.m_coordinate + coord;
		if ( tileModified.m_tileInfo.isValid() )
		{
			m_oldTileModifiedList.insert( TileModified( point, getTileInfo( m_mapInfo.getIndex(point), layerIndex ) ) );
			paintMap( point, tileModified.m_tileInfo );
		}
	}
	setIsShowSelection( false );
	QUndoCommand* command = new DrawCommand( this, layerIndex, m_oldTileModifiedList );
	m_undoStack->push( command );
}

void MapScene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mousePressEvent( event );
	setIsShowSelection( false );
	if( event->button() & Qt::LeftButton )
	{
		if ( eDrawTool::MOVE == m_parentWidget->m_drawTool )
		{
			m_view->viewport()->setCursor( Qt::ClosedHandCursor );
			m_parentWidget->disableShortcut( true );
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
			TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
			m_oldTileModifiedList.clear();

			// Implement Flood fill and Paint tiles
			QList<QPoint> readyToPaintTileCoords = editMapByFloodFill( currentIndex, coord );
			if ( readyToPaintTileCoords.isEmpty() )
			{
				return;
			}
			QPoint smallestPoint = readyToPaintTileCoords[0];
			for ( QPoint p : readyToPaintTileCoords )
			{
				smallestPoint.setX( qMin( smallestPoint.x(), p.x() ) );
				smallestPoint.setY( qMin( smallestPoint.y(), p.y() ) );
			}

			bool isDefault = true;
			int layerIndex = getCurrentLayerIndex();
			m_parentWidget->isDefalutBrush( isDefault );
			if ( isDefault )
			{
				QList<TileInfo> selectedTileList = getCurrentTiles();
				QSize regionSize = getSelectedTilesRegionSize();

				for( int i = 0; i < readyToPaintTileCoords.size(); ++i )
				{
					QPoint targetCoord = readyToPaintTileCoords[i];
					QPoint coord = QPoint( (targetCoord.x() - smallestPoint.x()) % regionSize.width(), (targetCoord.y() - smallestPoint.y()) % regionSize.height() );
					int index = coord.x() + coord.y() * regionSize.width();
					m_oldTileModifiedList.insert( TileModified( targetCoord, getTileInfo( m_mapInfo.getIndex( targetCoord ), layerIndex ) ) );
					paintMap( targetCoord, selectedTileList[index] );
				}
			}
			else
			{
				for( int i = 0; i < readyToPaintTileCoords.size(); ++i )
				{
					QList<TileModified> modifiedList;
					m_parentWidget->getPaintMapModified( modifiedList, readyToPaintTileCoords[i], eDrawTool::BRUSH );
					for( TileModified m : modifiedList )
					{
						m_oldTileModifiedList.insert( TileModified( m.m_coordinate, getTileInfo( m_mapInfo.getIndex( m.m_coordinate ), layerIndex ) ) );
						paintMap( m.m_coordinate, m.m_tileInfo );
					}
				}
			}
			QUndoCommand* command = new DrawCommand( this, layerIndex, m_oldTileModifiedList );
			m_undoStack->push( command );
		}
		else if ( eDrawTool::SHAPE == m_parentWidget->m_drawTool )
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
			m_lastPaintCoord = coord;
			m_startPos = mousePos;

			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 )
			{
				return;
			}
			TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
			m_oldTileModifiedList.clear();
			m_parentWidget->disableShortcut( true );

			int layerIndex = getCurrentLayerIndex();
			bool isDefault = true;
			m_parentWidget->isDefalutBrush( isDefault );
			if ( isDefault )
			{
				QList<TileInfo> selectedTileList = getCurrentTiles();

				if ( selectedTileList.empty() )
				{
					return;
				}
				TileInfo t = selectedTileList[0];
				m_oldTileModifiedList.insert( TileModified( coord, getTileInfo( m_mapInfo.getIndex( coord ), layerIndex ) ) );
				paintMap( coord, t );
			}
			else
			{
				QList<TileModified> modifiedList;
				m_parentWidget->getPaintMapModified( modifiedList, QPoint( coord.x(), coord.y() ), m_parentWidget->getCurrentDrawTool() );
				for( TileModified m : modifiedList )
				{
					m_oldTileModifiedList.insert( TileModified( m.m_coordinate, getTileInfo( m_mapInfo.getIndex( m.m_coordinate ), layerIndex ) ) );
					paintMap( m.m_coordinate, m.m_tileInfo );
				}
			}
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
			TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
			TileInfo currentTileInfo = tileLayer->getTileInfo( coord.x(), coord.y() );
			for( int i = 0; i < m_selectedTileItemList.size(); ++i )
			{
				if ( tileLayer->getTileInfo( i ) == currentTileInfo )
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
			m_oldTileModifiedList.clear();
			m_oldTileMarkerModifiedList.clear();
			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() || !m_layers[currentIndex]->getLayerInfo().isVisible() )
			{
				return;
			}
			
			m_parentWidget->disableShortcut( true );
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() )
			{
				return;
			}
			m_lastPaintCoord = QPoint( -1, -1 );
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
		else if ( eDrawTool::SHAPE == m_parentWidget->m_drawTool )
		{
			QPointF mousePos = event->scenePos();
			if( mousePos == QPointF() || m_selectedTileItemList.isEmpty() )
			{
				return;
			}
			QSizeF bound = QSizeF( m_mapInfo.getTileSize().width() * m_mapInfo.getMapSize().width(), m_mapInfo.getTileSize().height() * m_mapInfo.getMapSize().height() );
			if( mousePos.x() >= bound.width() || mousePos.y() >= bound.height() || mousePos.x() <= 0 || mousePos.y() <= 0 )
			{
				return;
			}
			QPoint coord = QPoint( qFloor( mousePos.x() / m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / m_mapInfo.getTileSize().height() ) );
			if ( m_lastPaintCoord.x() == coord.x() && m_lastPaintCoord.y() == coord.y() )
			{
				return;
			}

			m_lastPaintCoord = coord;
			QSize& tileSize = m_mapInfo.getTileSize();
			QPointF minPoint = QPointF( qMin( m_startPos.x(), mousePos.x() ), qMin( m_startPos.y(), mousePos.y() ) );
			QPointF maxPoint = QPointF( qMax( m_startPos.x(), mousePos.x() ), qMax( m_startPos.y(), mousePos.y() ) );

			m_selectedMinCoord = QPoint( qFloor( minPoint.x() / tileSize.width() ), qFloor( minPoint.y() / tileSize.height() ) );
			m_selectedMaxCoord = QPoint( qCeil( maxPoint.x() / tileSize.width() ), qCeil( maxPoint.y() / tileSize.height() ) );

			QSize regionSize = QSize( m_selectedMaxCoord.x() - m_selectedMinCoord.x(), m_selectedMaxCoord.y() - m_selectedMinCoord.y() );

			QSet<TileModified>::const_iterator i = m_oldTileModifiedList.constBegin();
			while( i != m_oldTileModifiedList.constEnd() ) {
				TileInfo tile = i->m_tileInfo;
				paintMap( i->m_coordinate, tile );
				++i;
			}

			QMap<QPoint, TileModified> selectedPointMap;
			bool isDefault = true;
			int layerIndex = getCurrentLayerIndex();
			m_parentWidget->isDefalutBrush( isDefault );
			if( isDefault )
			{
				QList<TileInfo> selectedTileList = getCurrentTiles();
				QSize tileRegionSize = getSelectedTilesRegionSize();
				for( int y = 0; y < regionSize.height(); ++y )
				{
					for( int x = 0; x < regionSize.width(); ++x )
					{
						QPoint targetCoord = m_selectedMinCoord + QPoint( x, y );

						QPoint tileCoord = QPoint( (targetCoord.x() - m_selectedMinCoord.x()) % tileRegionSize.width(), (targetCoord.y() - m_selectedMinCoord.y()) % tileRegionSize.height() );
						int index = tileCoord.x() + tileCoord.y() * tileRegionSize.width();

						m_oldTileModifiedList.insert( TileModified( targetCoord, getTileInfo( m_mapInfo.getIndex( targetCoord ), layerIndex ) ) );
						paintMap( targetCoord, selectedTileList[index] );
					}
				}
			}
			else
			{
				QList<TileInfo> selectedTileList = getCurrentTiles();
				QSize tileRegionSize = getSelectedTilesRegionSize();
				for( int y = 0; y < regionSize.height(); ++y )
				{
					for( int x = 0; x < regionSize.width(); ++x )
					{
						QPoint targetCoord = m_selectedMinCoord + QPoint( x, y );

						QList<TileModified> modifiedList;
						m_parentWidget->getPaintMapModified( modifiedList, QPoint( targetCoord.x(), targetCoord.y() ), eDrawTool::BRUSH );
						for( TileModified m : modifiedList )
						{
							m_oldTileModifiedList.insert( TileModified( m.m_coordinate, getTileInfo( m_mapInfo.getIndex( m.m_coordinate ), layerIndex ) ) );
							paintMap( m.m_coordinate, m.m_tileInfo );
						}
					}
				}
			}
		}
		// Draw or erase mode
		else if( eDrawTool::BRUSH == m_parentWidget->m_drawTool ||
			eDrawTool::ERASER == m_parentWidget->m_drawTool )
		{
			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() || !m_layers[currentIndex]->getLayerInfo().isVisible() )
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
			m_view->viewport()->setCursor( m_parentWidget->m_currentCursor );
			m_parentWidget->disableShortcut( false );
			return;
		}
		else if( eDrawTool::CURSOR == m_parentWidget->m_drawTool )
		{
			return;
		}
		else if ( eDrawTool::SHAPE == m_parentWidget->m_drawTool )
		{
			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() || !m_layers[currentIndex]->getLayerInfo().isVisible() )
			{
				return;
			}
			int layerIndex = getCurrentLayerIndex();

			TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_layers[currentIndex]);
			QUndoCommand* command = new DrawCommand( this, layerIndex, m_oldTileModifiedList );
			m_undoStack->push( command );
			m_parentWidget->disableShortcut( false );
		}
		// Draw or erase mode
		else if( eDrawTool::BRUSH == m_parentWidget->m_drawTool ||
			eDrawTool::ERASER == m_parentWidget->m_drawTool )
		{
			int currentIndex = getCurrentLayerIndex();
			if( currentIndex == -1 || m_layers[currentIndex]->getLayerInfo().isLock() || !m_layers[currentIndex]->getLayerInfo().isVisible() )
			{
				return;
			}
			int layerIndex = getCurrentLayerIndex();

			Layer* layer = m_layers[currentIndex];
			if( layer->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
			{
				QUndoCommand* command = new DrawCommand( this, layerIndex, m_oldTileModifiedList );
				m_undoStack->push( command );
			}
			else if( layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
			{
				QUndoCommand* command = new DrawMarkerCommand( this, layerIndex, m_oldTileMarkerModifiedList );
				m_undoStack->push( command );
			}
			m_parentWidget->disableShortcut( false );
		}
	}
	update();
}