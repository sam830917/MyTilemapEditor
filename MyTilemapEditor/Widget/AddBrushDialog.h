#pragma once

#include <QDialog>
#include <QBoxLayout>
#include "ui_AddBrushUI.h"

class AddBrushDialog : public QDialog
{
	Q_OBJECT

public:
	AddBrushDialog( QWidget* parent = Q_NULLPTR );

	void setAddLayout( QBoxLayout* widget );

public:
	Ui::AddBrushUI m_ui;

};