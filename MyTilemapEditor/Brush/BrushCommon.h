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

enum class eItemType
{
	STRING,
	TILE_INFO,
	TILE_INFO_LIST,

	UNKNOWN,
};

struct BrushType
{
	BrushType() {};
	BrushType( const QString& displayName, BrushConstructorFunction constructorFunction );

	const QString m_displayName;
	BrushConstructorFunction m_constructorFunction;
};

struct AddBrushItem
{
	eItemType m_type = eItemType::UNKNOWN;
	QString m_name;
	QWidget* m_widgetItem;
	QTreeWidgetItem* m_treeItem;
};

struct BrushFile
{
	BrushFile() {}
	BrushFile( Brush* brush, QString filePath ) :m_brush(brush), m_filePath(filePath) {}

	Brush* m_brush;
	QString m_filePath;
};

void createBrushUIItem( const QString& name, QString* val, QList<AddBrushItem*>& itemList );
void createBrushUIItem( const QString& name, TileInfo* val, QList<AddBrushItem*>& itemList );
void createBrushUIItem( const QString& name, QList<TileInfo>* val, QList<AddBrushItem*>& itemList );

Brush* copyBrush( Brush* referBrush );

bool saveBrushAsFile( Brush* brush, QString filePath );
Brush* loadBrush( const QString& brushFilePath );
bool isListType( eItemType type );