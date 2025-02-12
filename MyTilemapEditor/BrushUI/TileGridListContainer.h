#pragma once

#include "BrushUI/ListContainerBase.h"
#include "BrushUI/TileGridSelector.h"
#include "Core/TileInfo.h"

class TileSelector;
class TileGridSelector;

class TileGridListContainer : public ListContainerBase
{
public:
	TileGridListContainer();
	~TileGridListContainer();

	virtual void deleteChild( int index ) override;
	virtual void attachedTreeWidget() override;

	QList<TileGridSelector*> getTileGridSelectorList() const { return m_tileGridSelectorList; }

	void setGridType( eTileGridType gridType ) { m_gridType = gridType; }
	void addTileSelectorList();
	void appendToInitialList( TileInfo tile ) { m_initialTileList.push_back( tile ); }
	void appendToInitialList( QList<bool> states ) { m_initialStatesList.push_back( states ); }

private:
	QList<QWidget*> m_widget;
	QList<TileGridSelector*> m_tileGridSelectorList;
	QList<TileInfo> m_initialTileList;
	QList<QList<bool>> m_initialStatesList;
	eTileGridType m_gridType = eTileGridType::EDGE_AND_CORNER;
};