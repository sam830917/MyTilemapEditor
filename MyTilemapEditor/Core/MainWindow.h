#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include "Core/Config.h"
#include <QCloseEvent>
#include <QShortcut>
#include "Toolbar.h"
#include "Widget/ProjectWidget.h"
#include "Widget/WorkspaceWidget.h"
#include "Widget/TilesetWidget.h"
#include "Widget/LayerWidget.h"
#include "Widget/PropertiesWidget.h"
#include "Widget/MinimapWidget.h"
#include "Widget/BrushWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);

protected:
	void closeEvent( QCloseEvent* event );

private:
    void initialMenuBar();
    void initialToolBar();
    void initialStatusBar();
	void initialDockWidgets();
	void initialConnections();
	void initialShortcut();
	void restoreStates();

public slots:
	void updateToolBar();
	void changeDrawTool( QAction* action );
	void replaceRedoAction( QAction* action );
	void replaceUndoAction( QAction* action );
	void disableShortcut( bool isDisable );
	void closeCurrentProject( bool& isSuccess );

signals:
	void quit();

private:
	Config* m_config;
	QMenuBar* m_mainMenuBar;
	QMenu* m_windowsMenu;
	QMenu* m_fileMenu;
	QMenu* m_editMenu;

	// Actions
	QAction* m_tilesetAction;
	QAction* m_mapAction;
	QAction* m_saveAction;
	QAction* m_saveAllAction;
	QAction* m_projectNewAction;
	QAction* m_projectOpenAction;
	QAction* m_redoAction;
	QAction* m_undoAction;

	// Tool Actions
	QAction* m_cursorToolAction;
	QAction* m_moveToolAction;
	QAction* m_brushAction;
	QAction* m_eraserAction;
	QAction* m_bucketAction;
	QAction* m_magicWandAction;
	QAction* m_selectSameTileAction;

	QShortcut* m_eraseSelectedTilesShortcut;
	QShortcut* m_selectAllTilesShortcut;
	QShortcut* m_workspaceSwitchTabShortcut;
	QShortcut* m_workspaceCloseTabShortcut;
	QShortcut* m_newLayerShortcut;
	QShortcut* m_raiseLayerShortcut;
	QShortcut* m_lowerLayerShortcut;

    ToolBar* m_basicToolbar;
    ToolBar* m_paintToolToolbar;

	// Widgets
	WorkspaceWidget*	m_centralWidget;
	ProjectWidget*		m_projectWidget;
	TilesetWidget*		m_tilesetWidget;
	LayerWidget*		m_layerWidget;
	PropertiesWidget*	m_propertiesWidget;
	MinimapWidget*		m_minimapWidget;
	BrushWidget*		m_brushWidget;
};
