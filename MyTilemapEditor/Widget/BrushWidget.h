#pragma once

#include <QDockWidget>
#include <QListWidget>

QT_FORWARD_DECLARE_CLASS( QToolBar )
QT_FORWARD_DECLARE_CLASS( QBoxLayout )
QT_FORWARD_DECLARE_CLASS( QComboBox )
class Brush;

class BrushWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit BrushWidget( const QString& title, QWidget* parent = Q_NULLPTR );
	~BrushWidget();

	void addBrush( Brush* brush );

private:
	void initialToolbar();

public slots:
	void getCurrentBrush( Brush*& brush ) const;
	void createNewBrush();

private:
	QToolBar* m_toolbar;
	QAction* m_newBrushAction;
	QComboBox* m_brushListBox;
	QBoxLayout* m_layout;
	QListWidget* m_listWidget;
	QList<Brush*> m_brushList;
};