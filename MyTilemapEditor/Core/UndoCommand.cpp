#include "UndoCommand.h"
#include "Core/MapScene.h"
#include "Core/Layer.h"
#include "Widget/WorkspaceWidget.h"

DrawCommand::DrawCommand( MapScene* mapScene, int layerIndex, QSet<TileModified> oldTileModifiedList, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_mapScene(mapScene),
	m_index(layerIndex),
	m_oldTileModifiedList(oldTileModifiedList)
{
	if ( m_mapScene->m_layers[m_index]->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
	{
		TileLayer* tileLayer = dynamic_cast<TileLayer*>(m_mapScene->m_layers[m_index]);
		QSet<TileModified>::const_iterator i = m_oldTileModifiedList.constBegin();
		while( i != m_oldTileModifiedList.constEnd() ) {
			TileModified tileModified( i->m_coordinate, tileLayer->getTileInfo( i->m_coordinate.x(), i->m_coordinate.y() ) );
			m_newTileModifiedList.insert(tileModified);
			++i;
		}
	}
}

DrawCommand::~DrawCommand()
{
}

void DrawCommand::undo()
{
	QSet<TileModified>::const_iterator i = m_oldTileModifiedList.constBegin();
	while( i != m_oldTileModifiedList.constEnd() ) {
		m_mapScene->paintMap( i->m_coordinate, i->m_tileInfo, m_index );
		++i;
	}

	m_mapScene->m_parentWidget->markCurrentSceneForModified();
}

void DrawCommand::redo()
{
	QSet<TileModified>::const_iterator i = m_newTileModifiedList.constBegin();
	while( i != m_newTileModifiedList.constEnd() ) {
		m_mapScene->paintMap( i->m_coordinate, i->m_tileInfo, m_index );
		++i;
	}

	m_mapScene->m_parentWidget->markCurrentSceneForModified();
}

DrawMarkerCommand::DrawMarkerCommand( MapScene* mapScene, int layerIndex, QSet<TileMarkerModified> oldTileModifiedList, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_mapScene( mapScene ),
	m_index( layerIndex ),
	m_oldTileModifiedList( oldTileModifiedList )
{
	if( m_mapScene->m_layers[m_index]->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		MarkerLayer* markerLayer = dynamic_cast<MarkerLayer*>(m_mapScene->m_layers[m_index]);
		QSet<TileMarkerModified>::const_iterator i = m_oldTileModifiedList.constBegin();
		while( i != m_oldTileModifiedList.constEnd() ) 
		{
			TileMarkerModified tileModified( i->m_coordinate, markerLayer->IsMarked( i->m_coordinate.x(), i->m_coordinate.y() ) );
			m_newTileModifiedList.insert( tileModified );
			++i;
		}
	}
}

DrawMarkerCommand::~DrawMarkerCommand()
{

}

void DrawMarkerCommand::undo()
{
	QSet<TileMarkerModified>::const_iterator i = m_oldTileModifiedList.constBegin();
	while( i != m_oldTileModifiedList.constEnd() )
	{
		if( i->m_isMarked )
		{
			m_mapScene->paintMap( i->m_coordinate, TileInfo(), m_index );
		}
		else
		{
			m_mapScene->eraseMap( i->m_coordinate, m_index );
		}
		++i;
	}

	m_mapScene->m_parentWidget->markCurrentSceneForModified();
}

void DrawMarkerCommand::redo()
{
	QSet<TileMarkerModified>::const_iterator i = m_newTileModifiedList.constBegin();
	while( i != m_newTileModifiedList.constEnd() ) 
	{
		if ( i->m_isMarked )
		{
			m_mapScene->paintMap( i->m_coordinate, TileInfo(), m_index );
		}
		else
		{
			m_mapScene->eraseMap( i->m_coordinate, m_index );
		}
		++i;
	}

	m_mapScene->m_parentWidget->markCurrentSceneForModified();
}


LayerMoveCommand::LayerMoveCommand( MapScene* mapScene, int fromItemIndex, int toItemIndex, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_mapScene( mapScene ),
	m_indexA( fromItemIndex ),
	m_indexB( toItemIndex )
{
}

LayerMoveCommand::~LayerMoveCommand()
{
}

void LayerMoveCommand::undo()
{
	Layer* layerA = m_mapScene->m_layers[m_indexA];
	Layer* layerB = m_mapScene->m_layers[m_indexB];

	m_mapScene->m_layers[m_indexA]->setOrder( m_indexB );
	m_mapScene->m_layers[m_indexB]->setOrder( m_indexA );
	m_mapScene->m_layers[m_indexA] = layerB;
	m_mapScene->m_layers[m_indexB] = layerA;

	m_mapScene->update();
	m_mapScene->m_parentWidget->movedLayerOrder( m_indexB, m_indexA );
}

void LayerMoveCommand::redo()
{
	Layer* layerA = m_mapScene->m_layers[m_indexA];
	Layer* layerB = m_mapScene->m_layers[m_indexB];

	m_mapScene->m_layers[m_indexA]->setOrder( m_indexB );
	m_mapScene->m_layers[m_indexB]->setOrder( m_indexA );
	m_mapScene->m_layers[m_indexA] = layerB;
	m_mapScene->m_layers[m_indexB] = layerA;

	m_mapScene->update();
	m_mapScene->m_parentWidget->movedLayerOrder( m_indexA, m_indexB );
}

LayerAddCommand::LayerAddCommand( MapScene* mapScene, int index, const QString& name, eLayerType type, QUndoCommand* parent )
	: QUndoCommand( parent ),
	m_mapScene(mapScene),
	m_index(index),
	m_name(name),
	m_layerType(type)
{
	switch( m_layerType )
	{
	case eLayerType::TILE_LAYER:
		m_layer = new TileLayer( m_mapScene, m_index );
		break;
	case eLayerType::MARKER_LAYER:
		m_layer = new MarkerLayer( m_mapScene, m_index );
		break;
	default:
		break;
	}
	m_layer->setName( m_name );
}

LayerAddCommand::~LayerAddCommand()
{
}

void LayerAddCommand::undo()
{
	m_mapScene->m_layers.removeAt( m_index );
	switch( m_layerType )
	{
	case eLayerType::TILE_LAYER:
	{
		TileLayer* layer = dynamic_cast<TileLayer*>(m_layer);
		for( Tile* tile : layer->m_tileList )
		{
			m_mapScene->removeItem( tile );
		}
		break;
	}
	case eLayerType::MARKER_LAYER:
	{
		MarkerLayer* layer = dynamic_cast<MarkerLayer*>(m_layer);
		for( MarkerTile* tile : layer->m_tileList )
		{
			m_mapScene->removeItem( tile );
		}
		break;
	}
	default:
		break;
	}

	for( int i = m_index; i < m_mapScene->m_layers.size(); ++i )
	{
		m_mapScene->m_layers[i]->setOrder( i );
	}
	m_mapScene->update();
	m_mapScene->m_parentWidget->deletedLayer( m_index );
}

void LayerAddCommand::redo()
{
	m_mapScene->m_layers.insert( m_index, m_layer );
	switch( m_layerType )
	{
	case eLayerType::TILE_LAYER:
	{
		TileLayer* layer = dynamic_cast<TileLayer*>(m_layer);
		for( Tile* tile : layer->m_tileList )
		{
			m_mapScene->addItem( tile );
		}
		break;
	}
	case eLayerType::MARKER_LAYER:
	{
		MarkerLayer* layer = dynamic_cast<MarkerLayer*>(m_layer);
		for( MarkerTile* tile : layer->m_tileList )
		{
			m_mapScene->addItem( tile );
		}
		break;
	}
	default:
		break;
	}
	m_mapScene->m_parentWidget->addedNewLayer( m_index, m_name, m_layerType );
}

LayerDeleteCommand::LayerDeleteCommand( MapScene* mapScene, int index, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_mapScene( mapScene ),
	m_index( index )
{
	m_layer = m_mapScene->m_layers[m_index];
}

LayerDeleteCommand::~LayerDeleteCommand()
{
}

void LayerDeleteCommand::undo()
{
	m_mapScene->m_layers.insert( m_index, m_layer );
	if ( m_layer->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
	{
		TileLayer* layer = dynamic_cast<TileLayer*>(m_layer);
		for( Tile* tile : layer->m_tileList )
		{
			m_mapScene->addItem( tile );
		}
	}
	else if ( m_layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		MarkerLayer* layer = dynamic_cast<MarkerLayer*>(m_layer);
		for( MarkerTile* tile : layer->m_tileList )
		{
			m_mapScene->addItem( tile );
		}
	}

	// reorder z value
	for( int i = 0; i < m_mapScene->m_layers.size(); ++i )
	{
		m_mapScene->m_layers[i]->setOrder( i );
	}
	m_mapScene->m_parentWidget->addedNewLayerWithInfo( m_index, m_layer->getLayerInfo() );
}

void LayerDeleteCommand::redo()
{
	m_mapScene->m_layers.removeAt( m_index );
	if( m_layer->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
	{
		TileLayer* layer = dynamic_cast<TileLayer*>(m_layer);
		for( Tile* tile : layer->m_tileList )
		{
			m_mapScene->removeItem( tile );
		}
	}
	else if( m_layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		MarkerLayer* layer = dynamic_cast<MarkerLayer*>(m_layer);
		for( MarkerTile* tile : layer->m_tileList )
		{
			m_mapScene->removeItem( tile );
		}
	}

	// reorder z value
	for( int i = m_index; i < m_mapScene->m_layers.size(); ++i )
	{
		m_mapScene->m_layers[i]->setOrder( i );
	}
	m_mapScene->update();
	m_mapScene->m_parentWidget->deletedLayer( m_index );
}

LayerRenameCommand::LayerRenameCommand( MapScene* mapScene, int index, const QString& name, QUndoCommand* parent )
	: QUndoCommand( parent ),
	m_mapScene( mapScene ),
	m_index( index ),
	m_newName( name )
{
	m_oldName = m_mapScene->m_layers[index]->getLayerInfo().getNmae();
}

LayerRenameCommand::~LayerRenameCommand()
{
}

void LayerRenameCommand::undo()
{
	m_mapScene->m_layers[m_index]->setName( m_oldName );
	m_mapScene->m_parentWidget->renamedLayer( m_index, m_oldName );
}

void LayerRenameCommand::redo()
{
	m_mapScene->m_layers[m_index]->setName( m_newName );
	m_mapScene->m_parentWidget->renamedLayer( m_index, m_newName );
}

LayerColorChangeCommand::LayerColorChangeCommand( MapScene* mapScene, int index, const QColor& color, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_mapScene( mapScene ),
	m_index( index ),
	m_newColor( color )
{
	m_oldColor = m_mapScene->m_layers[index]->getLayerInfo().getColor();
}

LayerColorChangeCommand::~LayerColorChangeCommand()
{

}

void LayerColorChangeCommand::undo()
{
	m_mapScene->m_layers[m_index]->setColor( m_oldColor );
	Layer* layer = m_mapScene->m_layers[m_index];
	if( layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		MarkerLayer* markerLayer = dynamic_cast<MarkerLayer*>(layer);
		markerLayer->setColor( m_oldColor );
		for( int i = 0; i < markerLayer->m_tileList.size(); ++i )
		{
			markerLayer->m_tileList[i]->update();
		}
	}
	m_mapScene->m_parentWidget->changedColorLayer( m_index, m_oldColor );
}

void LayerColorChangeCommand::redo()
{
	m_mapScene->m_layers[m_index]->setColor( m_newColor );
	Layer* layer = m_mapScene->m_layers[m_index];
	if ( layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
	{
		MarkerLayer* markerLayer = dynamic_cast<MarkerLayer*>(layer);
		markerLayer->setColor(m_newColor);
		for ( int i = 0; i < markerLayer->m_tileList.size(); ++i )
		{
			markerLayer->m_tileList[i]->update();
		}
	}
	m_mapScene->m_parentWidget->changedColorLayer( m_index, m_newColor );
}

bool TileMarkerModified::operator==( const TileMarkerModified& compare ) const
{
	return compare.m_coordinate.x() == m_coordinate.x() && compare.m_coordinate.y() == m_coordinate.y();
}

uint qHash( const TileMarkerModified key )
{
	return key.m_coordinate.x() + key.m_coordinate.y();
}