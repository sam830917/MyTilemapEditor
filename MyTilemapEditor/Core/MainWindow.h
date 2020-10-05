#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QCloseEvent>
#include "Toolbar.h"
#include "Widget/ProjectWidget.h"
#include "Widget/WorkspaceWidget.h"
#include "Widget/TilesetWidget.h"
#include "Widget/LayerWidget.h"
#include "Widget/PropertiesWidget.h"
#include "Widget/MinimapWidget.h"

class MainWindow : public QMainWindow
{
	friend class ViewportEventFilter;
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

public slots:
	void updateToolBar();
	void changeDrawTool( QAction* action );
	void replaceRedoAction( QAction* action );
	void replaceUndoAction( QAction* action );
	void disableShortcut( bool isDisable );

private:
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

	QAction* m_cursorToolAction;
	QAction* m_moveToolAction;
	QAction* m_brushAction;
	QAction* m_eraserAction;

    ToolBar* m_basicToolbar;
    ToolBar* m_paintToolToolbar;

	// Widgets
	WorkspaceWidget*	m_centralWidget;
	ProjectWidget*		m_projectWidget;
	TilesetWidget*		m_tilesetWidget;
	LayerWidget*		m_layerWidget;
	PropertiesWidget*	m_propertiesWidget;
	MinimapWidget*		m_minimapWidget;
};
