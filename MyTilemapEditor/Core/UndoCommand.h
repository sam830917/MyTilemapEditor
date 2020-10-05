#pragma once

#include "Core/TileInfo.h"
#include <QUndoCommand>
#include <QList>

class MapScene;
class Tile;
class Layer;

class DrawCommand : public QUndoCommand
{
public:
	DrawCommand( QList<TileInfo> tileInfoList, QList<Tile*> tiles, QUndoCommand* parent = 0 );
	~DrawCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	QList<TileInfo> m_tileInfoBeforeList;
	QList<TileInfo> m_tileInfoAfterList;
	QList<Tile*> m_tiles;

};

class LayerMoveCommand : public QUndoCommand
{
	// Index A and Index B exchange position in the list.
public:
	LayerMoveCommand( MapScene* mapScene, int fromItemIndex, int toItemIndex, QUndoCommand* parent = 0 );
	~LayerMoveCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	MapScene* m_mapScene;
	int m_indexA = 0;
	int m_indexB = 0;
};

class LayerAddCommand : public QUndoCommand
{
	// Index A and Index B exchange position in the list.
public:
	LayerAddCommand( MapScene* mapScene, int index, QUndoCommand* parent = 0 );
	~LayerAddCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;

};