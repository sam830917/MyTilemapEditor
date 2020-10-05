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

LayerAddCommand::LayerAddCommand( MapScene* mapScene, int index, QUndoCommand* parent )
	: QUndoCommand( parent ),
	m_mapScene(mapScene),
	m_index(index)
{
}

LayerAddCommand::~LayerAddCommand()
{
}

void LayerAddCommand::undo()
{
	m_mapScene->m_parentWidget->deleteLayerFromIndex( m_index );
	m_mapScene->m_parentWidget->deletedLayer( m_index );
}

void LayerAddCommand::redo()
{
	m_mapScene->addNewLayer( m_index );
	m_mapScene->m_parentWidget->addedNewLayer( m_index );
}
