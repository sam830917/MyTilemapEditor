#pragma once

#include <QString>
#include <QBoxLayout>
#include <QTreeWidgetItem>

class Brush;

enum class eVariableType
{
	STRING,
	TILE,
	UNKNOWN,
};

typedef Brush* (*BrushConstructorFunction)();

#define BRUSH_TYPE( displayName, className ) \
	static Brush* className##_constructor() { return new className(); }\
	static BrushType className##_impl( displayName, className##_constructor );

#define CREATE_BASIC_ITEM Brush::createAddDialogItem();

#define CREATE_ITEM( lableName, variable, AddBrushItemList ) \
	switch ( getVariableType( variable ) ) \
	{ \
		case eVariableType::STRING: \
		{ \
			break; \
		} \
		case eVariableType::TILE: \
		{ \
			CREATE_TILE_ITEM( lableName, variable, AddBrushItemList ) \
			break; \
		} \
		default: \
		{ \
			break;\
		} \
	}

#define CREATE_TILE_ITEM( lableName, variable, AddBrushItemList ) \
	AddBrushItem* variable##_tileItem = new AddBrushItem();\
	TileSelector* variable##_tileSelector = new TileSelector( QSize( 50, 50 ) );\
	QObject::connect( variable##_tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo ) { this->variable = tileInfo; } );\
	variable##_tileItem->m_name = lableName;\
	variable##_tileItem->m_widgetItem = variable##_tileSelector;\
	AddBrushItemList.push_back( variable##_tileItem );

struct BrushType
{
	BrushType() {};
	BrushType( const QString& displayName, BrushConstructorFunction constructorFunction );

	const QString m_displayName;
	BrushConstructorFunction m_constructorFunction;
};

struct AddBrushItem
{
	QString m_name;
	QWidget* m_widgetItem;
};

template<typename T>
eVariableType getVariableType( const T& );