#pragma once

#include <QDialog>
#include <QBoxLayout>
#include "ui_AddBrushUI.h"
#include "Brush/BrushCommon.h"

class AddBrushDialog : public QDialog
{
	Q_OBJECT

public:
	AddBrushDialog( QWidget* parent = Q_NULLPTR );

	void addItem( QList<AddBrushItem*> items );

public:
	Ui::AddBrushUI m_ui;

};