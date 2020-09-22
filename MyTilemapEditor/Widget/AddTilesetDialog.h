#pragma once

#include <QDialog>
#include "ui_AddTilesetUI.h"
#include "../Core/Tileset.h"

QT_FORWARD_DECLARE_CLASS( QPushButton )
QT_FORWARD_DECLARE_CLASS( QGraphicsView )

class AddTilesetDialog : public QDialog
{
	Q_OBJECT

public:
	AddTilesetDialog( QWidget* parent = Q_NULLPTR );

	void updateTilePreview();
	Tileset* getResult();

private slots:
	void saveTileset();
	void addImageFile();
	void checkTileSizeAndUpdate( int val );

public:
	Ui::AddTilesetUI m_ui;
	QPixmap* m_tilesetImage;
	QString m_imagePath;
	QString m_saveFilePath;
	QString m_tilesetName;

};