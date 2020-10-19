#pragma once

#include <QString>
#include <QBoxLayout>
#include <QTreeWidgetItem>
#include <QList>
#include "Core/TileInfo.h"

class Brush;

typedef Brush* (*BrushConstructorFunction)();

#define BRUSH_TYPE( displayName, className ) \
	static Brush* className##_constructor() { return new className(); }\
	static BrushType className##_impl( displayName, className##_constructor );

#define CREATE_BASIC_ITEM Brush::createAddDialogItem();

#define CREATE_ITEM( lableName, variable, addBrushItemList ) createBrushUIItem( lableName, &variable, addBrushItemList );

#define CREATE_STRING_ITEM( lableName, variable, AddBrushItemList );

#define CREATE_TILE_ITEM( lableName, variable, AddBrushItemList );

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

void createBrushUIItem( const QString& name, QString* val, QList<AddBrushItem*>& itemList );
void createBrushUIItem( const QString& name, TileInfo* val, QList<AddBrushItem*>& itemList );