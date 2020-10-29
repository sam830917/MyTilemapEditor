#pragma once

#include "Core/ListContainerBase.h"
#include "Core/TileInfo.h"

class TileSelector;

class TileInfoListContainer : public ListContainerBase
{
public:
	TileInfoListContainer();
	~TileInfoListContainer();

	virtual void deleteChild( int index ) override;
	virtual void attachedTreeWidget() override;

	QList<TileSelector*> getTileSelectorList() const { return m_tileSelectorList; }

	void addTileSelectorList();
	void appendToInitialList( TileInfo tile ) { m_initialTileList.push_back(tile); }

private:
	QList<TileSelector*> m_tileSelectorList;
	QList<TileInfo> m_initialTileList;
};