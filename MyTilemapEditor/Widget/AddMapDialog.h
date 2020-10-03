#pragma once

#include <QDialog>
#include "ui_AddMapUI.h"
#include "Core/MapInfo.h"

class AddMapDialog : public QDialog
{
	Q_OBJECT

public:
	AddMapDialog( QWidget* parent = Q_NULLPTR );

	MapInfo* getResult() const;
private slots:
	void saveMap();

public:
	Ui::AddMapUI m_ui;
	QString m_filePath;

};