#include "Core/TilePalette.hpp"
#include "Core/Tileset.h"
#include "Widget/AddTilesetDialog.h"
#include "Utils/ProjectCommon.h"
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QtMath>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollBar>

extern QList<TileInfo> g_currentTileInfos;
extern QSize g_selectedTilesRegionSize;

TilePalette::TilePalette( Tileset* tileset, QObject* parent /*= Q_NULLPTR*/ )
	:QGraphicsScene( parent ),
	m_tileset( tileset )
{
	QGraphicsRectItem* bg = new QGraphicsRectItem();
	QColor bgColor( 0, 0, 0, 50 );

	bg->setBrush( QBrush( bgColor ) );
	bg->setPen( QPen( bgColor ) );
	bg->setRect( 0, 0, tileset->getImage()->size().width(), tileset->getImage()->size().height() );
	QGraphicsPixmapItem* imgItem = new QGraphicsPixmapItem( *tileset->getImage() );
	addItem( bg );
	addItem( imgItem );

	// draw grid
	QSize tileSize = tileset->getTileSize();
	QSize size = tileset->getImage()->size();
	int vLineCount = size.width() / tileSize.width();
	int hLineCount = size.height() / tileSize.height();

	QPen linePen( bgColor );
	linePen.setWidth( 2 );
	for( int v = 1; v < vLineCount; ++v )
	{
		addLine( v * tileSize.width(), 0, v * tileSize.width(), size.height(), linePen );
	}
	for( int h = 1; h < hLineCount; ++h )
	{
		addLine( 0, h * tileSize.height(), size.width(), h * tileSize.height(), linePen );
	}

	m_selectFrame = new QGraphicsRectItem();
	QBrush brush( QColor( 0, 0, 255, 70 ) );
	QPen redPen( Qt::red );
	redPen.setWidth( 2 );
	m_selectFrame->setRect( 0, 0, tileset->getTileSize().width(), tileset->getTileSize().height() );
	m_selectFrame->setBrush( brush );
	m_selectFrame->setPen( redPen );
	addItem( m_selectFrame );
	m_selectedTileInfos.push_back( TileInfo( m_tileset, 0 ) );
}

void TilePalette::updateGlobalTilePalette()
{
	if( !m_updateGlobalTilePalette )
	{
		return;
	}
	g_selectedTilesRegionSize = m_selectedRegionSize;
	g_currentTileInfos.clear();
	for( TileInfo tileInfo : m_selectedTileInfos )
	{
		g_currentTileInfos.push_back( tileInfo );
	}
}

void TilePalette::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mousePressEvent( event );
	if( event->button() & Qt::LeftButton )
	{
		QPointF mousePos = event->pos();
		if( mousePos == QPointF() )
		{
			return;
		}
		QSize size = m_tileset->getTileSize();
		QPoint coord = QPoint( qFloor( mousePos.x() / size.width() ), qFloor( mousePos.y() / size.height() ) );
		m_startPos = mousePos;
		// 		qDebug() << m_startPos.x() << "," << m_startPos.y();

		QPoint startPos = QPoint( coord.x() * size.width(), coord.y() * size.height() );
		m_selectFrame->setRect( startPos.x(), startPos.y(), size.width(), size.height() );
		m_selectedRegionSize = QSize( 1, 1 );

		if( !m_updateGlobalTilePalette )
		{
			m_currentIndex = m_tileset->getTileIndex( coord );
		}
		else
		{
			m_selectedTileInfos.clear();
			m_selectedTileInfos.push_back( TileInfo( m_tileset, m_tileset->getTileIndex( coord ) ) );
			updateGlobalTilePalette();
		}
	}
	update();
}

void TilePalette::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
	QGraphicsScene::mouseMoveEvent( event );
	if( event->buttons() & Qt::LeftButton )
	{
		if( m_selectMode == ePaletteSelectMode::PALETTE_SINGLE_SELECT )
		{
			return;
		}
		QPointF mousePos = event->scenePos();
		if( mousePos == QPointF() )
		{
			return;
		}
		QSize size = m_tileset->getTileSize();
		QSize coordTileSize = m_tileset->getCoordTileSize();
		QPointF minPoint = QPointF( qMin( m_startPos.x(), mousePos.x() ), qMin( m_startPos.y(), mousePos.y() ) );
		QPointF maxPoint = QPointF( qMax( m_startPos.x(), mousePos.x() ), qMax( m_startPos.y(), mousePos.y() ) );
		QPoint selectedMinCoord = QPoint( qMax( 0, qFloor( minPoint.x() / size.width() ) ), qMax( 0, qFloor( minPoint.y() / size.height() ) ) );
		QPoint selectedMaxCoord = QPoint( qMin( coordTileSize.width(), qCeil( maxPoint.x() / size.width() ) ), qMin( coordTileSize.height(), qCeil( maxPoint.y() / size.height() ) ) );
		QPoint startPos = QPoint( selectedMinCoord.x() * size.width(), selectedMinCoord.y() * size.height() );
		QPoint endPos = QPoint( selectedMaxCoord.x() * size.width(), selectedMaxCoord.y() * size.height() );

		QSize regionSize = QSize( endPos.x() - startPos.x(), endPos.y() - startPos.y() );
		m_selectFrame->setRect( startPos.x(), startPos.y(), regionSize.width(), regionSize.height() );

		m_selectedTileInfos.clear();
		QSize regionSizeInCoord = QSize( selectedMaxCoord.x() - selectedMinCoord.x(), selectedMaxCoord.y() - selectedMinCoord.y() );
		m_selectedRegionSize = regionSizeInCoord;

		for( int y = 0; y < regionSizeInCoord.height(); ++y )
		{
			for( int x = 0; x < regionSizeInCoord.width(); ++x )
			{
				m_selectedTileInfos.push_back( TileInfo( m_tileset, m_tileset->getTileIndex( selectedMinCoord + QPoint( x, y ) ) ) );
			}
		}
		m_currentIndex = m_selectedTileInfos[0].getIndex();
		updateGlobalTilePalette();
	}
	update();
}