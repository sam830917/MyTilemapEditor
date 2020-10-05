#include "UndoCommand.h"
#include "Core/MapScene.h"
#include "Core/Layer.h"
#include "Widget/WorkspaceWidget.h"

DrawCommand::DrawCommand( QList<TileInfo> tileInfoList, QList<Tile*> tiles, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_tileInfoBeforeList(tileInfoList),
	m_tiles(tiles)
{
	for ( Tile* t : m_tiles )
	{
		m_tileInfoAfterList.push_back( t->getTileInfo() );
	}
}

DrawCommand::~DrawCommand()
{
}

void DrawCommand::undo()
{
	for ( int i = 0; i < m_tiles.size(); ++i )
	{
		m_tiles[i]->setTileInfo( m_tileInfoBeforeList[i] );
		m_tiles[i]->update();
	}
	m_tiles[0]->m_mapScene->m_parentWidget->markCurrentSceneForModified();
}

void DrawCommand::redo()
{
	for( int i = 0; i < m_tiles.size(); ++i )
	{
		m_tiles[i]->setTileInfo( m_tileInfoAfterList[i] );
		m_tiles[i]->update();
	}
	m_tiles[0]->m_mapScene->m_parentWidget->markCurrentSceneForModified();
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

LayerAddCommand::LayerAddCommand( MapScene* mapScene, int index, const QString& name, QUndoCommand* parent )
	: QUndoCommand( parent ),
	m_mapScene(mapScene),
	m_index(index),
	m_name(name)
{
}

LayerAddCommand::~LayerAddCommand()
{
}

void LayerAddCommand::undo()
{
	Layer* layer = m_mapScene->m_layers[m_index];
	m_mapScene->m_layers.removeAt( m_index );
	delete layer;
	for( int i = m_index; i < m_mapScene->m_layers.size(); ++i )
	{
		m_mapScene->m_layers[i]->setOrder( i );
	}
	m_mapScene->update();
	m_mapScene->m_parentWidget->deletedLayer( m_index );
}

void LayerAddCommand::redo()
{
	Layer* layer = m_mapScene->addNewLayer( m_index );
	layer->setName(m_name);
	m_mapScene->m_parentWidget->addedNewLayer( m_index, m_name );
}

LayerDeleteCommand::LayerDeleteCommand( MapScene* mapScene, int index, QUndoCommand* parent /*= 0 */ )
	: QUndoCommand( parent ),
	m_mapScene( mapScene ),
	m_index( index )
{
}

LayerDeleteCommand::~LayerDeleteCommand()
{
}

void LayerDeleteCommand::undo()
{
	Layer* newLayer = new Layer( m_mapScene, m_index );
	newLayer->setLayerInfo( m_layerInfo );
	m_mapScene->m_layers.insert( m_index, newLayer );
	for( int i = 0; i < newLayer->m_tileList.size(); ++i )
	{
		newLayer->m_tileList[i]->m_tileInfo = m_tileInfoList[i];
	}

	// reorder z value
	for( int i = 0; i < m_mapScene->m_layers.size(); ++i )
	{
		m_mapScene->m_layers[i]->setOrder( i );
	}
	m_mapScene->m_parentWidget->addedNewLayerWithInfo( m_index, m_layerInfo );
}

void LayerDeleteCommand::redo()
{
	Layer* layer = m_mapScene->m_layers[m_index];
	for( Tile* t : layer->m_tileList )
	{
		m_tileInfoList.push_back( t->getTileInfo() );
	}
	m_layerInfo = layer->getLayerInfo();
	m_mapScene->m_layers.removeAt( m_index );
	delete layer;
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
