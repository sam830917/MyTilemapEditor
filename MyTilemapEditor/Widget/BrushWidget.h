#pragma once

#include <QDockWidget>
#include <QListWidget>
#include "Brush/BrushCommon.h"
#include "Utils/ProjectCommon.h"

QT_FORWARD_DECLARE_CLASS( QToolBar )
QT_FORWARD_DECLARE_CLASS( QBoxLayout )
QT_FORWARD_DECLARE_CLASS( QComboBox )
QT_FORWARD_DECLARE_CLASS( QPoint )
class Brush;
class BrushParser;

class BrushWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit BrushWidget( const QString& title, QWidget* parent = Q_NULLPTR );
	~BrushWidget();

private:
	void initialToolbar();
	void initialBrushFile();

public slots:
	void getPaintMapModified( QList<TileModified>& modifiredList, const QPoint& point, eDrawTool tool );
	void createNewBrush();
	void editBrush( QListWidgetItem* item );
	void addBrush( const QString& filePath );

private:
	QToolBar* m_toolbar;
	QAction* m_newBrushAction;
	QComboBox* m_brushListBox;
	QBoxLayout* m_layout;
	QListWidget* m_listWidget;
	BrushParser* m_brushParser;
};