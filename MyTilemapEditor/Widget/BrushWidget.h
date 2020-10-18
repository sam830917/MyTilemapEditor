#pragma once

#include <QDockWidget>
#include <QListWidget>

class Brush;

class BrushWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit BrushWidget( const QString& title, QWidget* parent = Q_NULLPTR );
	~BrushWidget();

	void addBrush( Brush* brush );

public slots:
	void getCurrentBrush( Brush*& brush ) const;

private:
	QListWidget* m_listWidget;
	QList<Brush*> m_brushList;
};