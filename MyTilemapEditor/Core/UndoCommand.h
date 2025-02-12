#pragma once

#include "Core/TileInfo.h"
#include "Core/LayerInfo.h"
#include "Core/Layer.h"
#include "Brush/BrushCommon.h"
#include <QUndoCommand>
#include <QList>

class MapScene;
class Tile;
class TileLayer;

struct TileMarkerModified
{
	TileMarkerModified( QPoint coordinate, bool isMarked ):m_coordinate( coordinate ), m_isMarked( isMarked ) {}

	QPoint m_coordinate;
	bool m_isMarked = false;

	bool operator==( const TileMarkerModified& compare ) const;
};
uint qHash( const TileMarkerModified key );

//--------------------------------------------------------------------------------------------------------------------------
class DrawCommand : public QUndoCommand
{
public:
	DrawCommand( MapScene* mapScene, int layerIndex, QSet<TileModified> oldTileModifiedList, QUndoCommand* parent = 0 );
	~DrawCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;
	QSet<TileModified> m_oldTileModifiedList;
	QSet<TileModified> m_newTileModifiedList;
};

class DrawMarkerCommand : public QUndoCommand
{
public:
	DrawMarkerCommand( MapScene* mapScene, int layerIndex, QSet<TileMarkerModified> oldTileModifiedList, QUndoCommand* parent = 0 );
	~DrawMarkerCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;
	QSet<TileMarkerModified> m_oldTileModifiedList;
	QSet<TileMarkerModified> m_newTileModifiedList;
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
	Layer* m_layer;
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

class LayerColorChangeCommand : public QUndoCommand
{
	// Index A and Index B exchange position in the list.
public:
	LayerColorChangeCommand( MapScene* mapScene, int index, const QColor& color, QUndoCommand* parent = 0 );
	~LayerColorChangeCommand();

	virtual void undo() override;
	virtual void redo() override;
private:
	int m_index = 0;
	MapScene* m_mapScene;
	QColor m_oldColor;
	QColor m_newColor;
};