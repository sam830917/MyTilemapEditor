#pragma once

#include "Core/MapScene.h"
#include "Core/MapInfo.h"
#include "Core/LayerInfo.h"
#include <QWidget>
#include <QTabWidget>
#include <QList>

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
	void insertMap( MapInfo mapInfo );
	void closeTab( int index );
	void setDrawTool( eDrawTool drawTool );
	void changeTab( int index );
	void changeMapScale( const QString& text );
	void nextTab();
	void closeCurrentTab();

	void saveCurrentMap();
	void saveAllMaps();
	void saveMap( int tabIndex );
	void getTabCount( int& tabCount );

signals:
	void updateRedo( QAction* action );
	void updateUndo( QAction* action );
	void disableShortcut( bool isDisable );
	void tabFocusChanged( int index );
	void closeTabSuccessfully( int index );

public:
	QPushButton* m_newProjectButton;
	QPushButton* m_openProjectButton;
	QTabWidget* m_mapTabWidget;
	QList<MapScene*> m_mapSceneList;
	eDrawTool m_drawTool = eDrawTool::MOVE;
};