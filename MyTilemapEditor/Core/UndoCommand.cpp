#include "UndoCommand.h"
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
	m_tiles[0]->m_mapScene->m_parentWidget->modifiedCurrentScene();
}

void DrawCommand::redo()
{
	for( int i = 0; i < m_tiles.size(); ++i )
	{
		m_tiles[i]->setTileInfo( m_tileInfoAfterList[i] );
		m_tiles[i]->update();
	}
	m_tiles[0]->m_mapScene->m_parentWidget->modifiedCurrentScene();
}
