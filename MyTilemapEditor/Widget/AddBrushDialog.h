#pragma once

#include <QDialog>
#include <QBoxLayout>
#include "ui_AddBrushUI.h"
#include "Brush/BrushCommon.h"

class Brush;
class BrushParser;

class AddBrushDialog : public QDialog
{
	Q_OBJECT

public:
	AddBrushDialog( QWidget* parent = Q_NULLPTR );

	void addItem( QList<AddBrushItem*> items );
	void setFilePath( const QString& filePath ) { m_brushFilePath = filePath; }

public slots:
	void saveBrush();

public:
	Ui::AddBrushUI m_ui;
	QList<AddBrushItem*> m_brushUI;
	QString m_brushFilePath;
	QString m_name = "New Brush";
	BrushParser* m_brushParser;
	bool m_isModify = false;
	int m_brushIndex = -1;
};