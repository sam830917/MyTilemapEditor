#pragma once

#include "../Core/MapScene.h"
#include "../Core/MapInfo.h"
#include <QWidget>
#include <QTabWidget>

QT_FORWARD_DECLARE_CLASS( QPushButton )

enum class eDrawTool
{
	MOVE,
	BRUSH,
	ERASER
};
Q_DECLARE_METATYPE( eDrawTool );

class WorkspaceWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WorkspaceWidget( QWidget* parent = Q_NULLPTR );

	void disableTabWidget( bool disable ) const;
	void modifiedCurrentScene();
	bool isReadyToClose();

	eDrawTool getCurrentDrawTool() { return m_drawTool; }

public slots:
	void addMap();
	void insertMap( MapInfo* mapInfo );
	void closeTab( int index );
	void setDrawTool( eDrawTool drawTool );
	void changeTab( int index );

	void saveCurrentMap();
	void saveAllMaps();
	void saveMap( int tabIndex );

signals:
	void updateRedo( QAction* action );
	void updateUndo( QAction* action );
	void disableShortcut( bool isDisable );

public:
	QPushButton* m_newProjectButton;
	QPushButton* m_openProjectButton;
	QTabWidget* m_mapTabWidget;
	QList<MapScene*> m_mapSceneList;
	eDrawTool m_drawTool = eDrawTool::MOVE;
};