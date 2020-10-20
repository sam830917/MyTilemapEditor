#pragma once

#include <QDialog>
#include <QBoxLayout>
#include "ui_AddBrushUI.h"
#include "Brush/BrushCommon.h"

class Brush;

class AddBrushDialog : public QDialog
{
	Q_OBJECT

public:
	AddBrushDialog( QWidget* parent = Q_NULLPTR );

	void addItem( QList<AddBrushItem*> items );
	void setBrush( Brush* brush ) { m_brushFile.m_brush = brush; }
	void setBrushFile( BrushFile brushFile ) { m_brushFile = brushFile; }

public slots:
	void saveBrush();

public:
	Ui::AddBrushUI m_ui;
	BrushFile m_brushFile;
};