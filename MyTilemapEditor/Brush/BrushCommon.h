#pragma once

#include <QString>
#include <QBoxLayout>

class Brush;

typedef Brush* (*BrushConstructorFunction)();

#define BRUSH_TYPE( displayName, className ) \
	static Brush* className##_constructor() { return new className(); }\
	static BrushType className##_impl( displayName, className##_constructor );

#define CREATE_BASIC_LAYOUT Brush::createAddDialogUI();

struct BrushType
{
	BrushType() {};
	BrushType( const QString& displayName, BrushConstructorFunction constructorFunction );

	const QString m_displayName;
	BrushConstructorFunction m_constructorFunction;
};