#pragma once

#include "Core/TileInfo.h"
#include "Core/LayerInfo.h"
#include "Core/Layer.h"
#include <QUndoCommand>
#include <QList>

class MapScene;
class Tile;
class TileLayer;

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
	LayerAddCommand( MapScene* mapScene, int index, const QString& name, eLayerType type, QUndoCommand* parent = 0 );
	~LayerAddCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;
	QString m_name;
	Layer* m_layer;
	eLayerType m_layerType;
};

class LayerDeleteCommand : public QUndoCommand
{
	// Index A and Index B exchange position in the list.
public:
	LayerDeleteCommand( MapScene* mapScene, int index, QUndoCommand* parent = 0 );
	~LayerDeleteCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;
	TileLayer* m_layer;
};

class LayerRenameCommand : public QUndoCommand
{
	// Index A and Index B exchange position in the list.
public:
	LayerRenameCommand( MapScene* mapScene, int index, const QString& name, QUndoCommand* parent = 0 );
	~LayerRenameCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;
	QString m_oldName;
	QString m_newName;
};