#pragma once

#include "MapScene.h"
#include <QUndoCommand>
#include <QList>

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