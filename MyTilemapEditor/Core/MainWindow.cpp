#include "MainWindow.h"
#include "Widget/AddTilesetDialog.h"
#include "Core/Tileset.h"
#include "Utils/ProjectCommon.h"
#include <QTimer>
#include <QPushButton>
#include <QEvent>
#include <QStatusBar>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	m_config = new Config();
	setWindowIcon( QIcon( ":/MainWindow/app-icon.ico" ) );

	initialMenuBar();
	initialToolBar();
	initialDockWidgets();
	initialShortcut();
	initialConnections();
	initialStatusBar();
	restoreStates();
}

void MainWindow::closeEvent( QCloseEvent* event )
{
	bool isAllReadyToClose = true;
	closeCurrentProject( isAllReadyToClose );

	if ( isAllReadyToClose )
	{
		QString projectFilePath = getProjectFilePath();
		m_config->set( "projectFilePath", projectFilePath );
		m_config->set( "windowState", saveState() );
		m_config->set( "geometry", saveGeometry() );
		QStringList openingMapPathList = m_centralWidget->getOpeningMapFilePath();
		m_config->set( "openingMaps", openingMapPathList );
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::initialMenuBar()
{
	m_mainMenuBar = new QMenuBar(this);
	setMenuBar( m_mainMenuBar );
	m_fileMenu = m_mainMenuBar->addMenu( "File" );

	m_projectNewAction = new QAction( tr( "New Project" ), this );
	m_fileMenu->addAction( m_projectNewAction );
	m_projectOpenAction = new QAction( tr( "Open Project" ), this );
	m_fileMenu->addAction( m_projectOpenAction );
	m_fileMenu->addSeparator();

	QMenu* newMenu = m_fileMenu->addMenu( "New" );
	m_tilesetAction = new QAction( tr( "Tileset" ), this );
	m_mapAction = new QAction( tr( "Map" ), this );
	newMenu->addAction( m_tilesetAction );
	newMenu->addAction( m_mapAction );
	m_tilesetAction->setDisabled(true);
	m_mapAction->setDisabled(true);

	m_saveAction = new QAction( tr( "Save" ), this );
	m_saveAction->setIcon( QIcon(":/MainWindow/Icon/save-file.png") );
	m_saveAction->setDisabled( true );
	m_saveAction->setToolTip( tr( "Save (Ctrl+S)" ) );
	m_saveAllAction = new QAction( tr( "Save All" ), this );
	m_saveAllAction->setIcon( QIcon( ":/MainWindow/Icon/save-all-files.png" ) );
	m_saveAllAction->setDisabled( true );
	m_saveAllAction->setToolTip( tr( "Save All (Ctrl+Shift+S)" ) );
	m_fileMenu->addAction( m_saveAction );
	m_fileMenu->addAction( m_saveAllAction );
	m_fileMenu->addSeparator();
	m_fileMenu->addAction( tr( "&Quit" ), this, &MainWindow::quit );

	m_editMenu = m_mainMenuBar->addMenu( "Edit" );
	m_undoAction = new QAction( QIcon( ":/MainWindow/Icon/undo.png" ), tr( "&Undo" ) );
	m_undoAction->setToolTip( tr( "Undo (Ctrl+Z)" ) );
	m_undoAction->setDisabled(true);
	m_editMenu->addAction( m_undoAction );
	m_redoAction = new QAction( QIcon( ":/MainWindow/Icon/redo.png" ), tr( "&Redo" ) );
	m_redoAction->setToolTip( tr( "Redo (Ctrl+Y)" ) );
	m_redoAction->setDisabled(true);
	m_editMenu->addAction( m_redoAction );

	// Windows
	m_windowsMenu = m_mainMenuBar->addMenu( "Windows" );
}

void MainWindow::initialToolBar()
{
	m_basicToolbar = new ToolBar( tr( "basic" ), this );
	m_basicToolbar->setDisabled(true);
	addToolBar( m_basicToolbar );

	QMenu* basicMenu = new QMenu("New", m_basicToolbar);
	basicMenu->setIcon( QIcon(":/MainWindow/Icon/new-file.png") );
	m_basicToolbar->addAction( basicMenu->menuAction() );
	basicMenu->addAction( m_tilesetAction );
	basicMenu->addAction( m_mapAction );
	m_basicToolbar->addAction( m_saveAction );
	m_basicToolbar->addAction( m_saveAllAction );
	m_basicToolbar->addAction( m_undoAction );
	m_basicToolbar->addAction( m_redoAction );

	m_paintToolToolbar = new ToolBar( tr( "paint tool" ), this );
	addToolBar( m_paintToolToolbar );

	m_cursorToolAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/cursor.png" ), tr( "&Cursor (C)" ) );
	m_cursorToolAction->setData( QVariant::fromValue( eDrawTool::CURSOR ) );
	m_moveToolAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/move.png" ), tr( "&Move (V)" ) );
	m_moveToolAction->setData( QVariant::fromValue( eDrawTool::MOVE ) );
	m_brushAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/pencil.png" ), tr( "&Brush (B)" ) );
	m_eraserAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/eraser.png" ), tr( "&Eraser (E)" ) );
	m_bucketAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/bucket.png" ), tr( "&Bucket (G)" ) );
	m_magicWandAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/magic-wand.png" ), tr( "&Magic Wand (W)" ) );
	m_selectSameTileAction = m_paintToolToolbar->addNewAction( QIcon( ":/MainWindow/Icon/same-tile.png" ), tr( "&Select Same Tile (S)" ) );

	QActionGroup* alignmentGroup = new QActionGroup( this );
	alignmentGroup->addAction( m_brushAction );
	alignmentGroup->addAction( m_eraserAction );
	alignmentGroup->addAction( m_bucketAction );
	alignmentGroup->addAction( m_magicWandAction );
	alignmentGroup->addAction( m_selectSameTileAction );
	alignmentGroup->addAction( m_moveToolAction );
	alignmentGroup->addAction( m_cursorToolAction );
	m_brushAction->setCheckable(true);
	m_eraserAction->setCheckable(true);
	m_moveToolAction->setCheckable(true);
	m_cursorToolAction->setCheckable(true);
	m_bucketAction->setCheckable(true);
	m_magicWandAction->setCheckable(true);
	m_selectSameTileAction->setCheckable(true);

	m_cursorToolAction->setChecked(true);

	m_brushAction->setData( QVariant::fromValue( eDrawTool::BRUSH ) );
	m_eraserAction->setData( QVariant::fromValue( eDrawTool::ERASER ) );
	m_bucketAction->setData( QVariant::fromValue( eDrawTool::BUCKET ) );
	m_magicWandAction->setData( QVariant::fromValue( eDrawTool::MAGIC_WAND ) );
	m_selectSameTileAction->setData( QVariant::fromValue( eDrawTool::SELECT_SAME_TILE ) );
	connect( alignmentGroup, &QActionGroup::triggered, this, &MainWindow::changeDrawTool );
}

void MainWindow::initialStatusBar()
{
	//QList<QString> scaleTextList;
	//scaleTextList<< "50 %" << "100 %" << "200 %";
	//QComboBox* mapScaleBox = new QComboBox;
	//mapScaleBox->addItems( scaleTextList );
	//mapScaleBox->setCurrentIndex(1);
	////mapScaleBox->setEditable( true );
	////mapScaleBox->setEditText( "100 %" );
	//mapScaleBox->setValidator( new QIntValidator() );
	//statusBar()->addPermanentWidget( mapScaleBox );
	//connect( mapScaleBox, SIGNAL( currentIndexChanged(const QString &) ), m_centralWidget, SLOT( changeMapScale( const QString & ) ) );
}

void MainWindow::initialDockWidgets()
{
	m_centralWidget = new WorkspaceWidget( this );
	m_centralWidget->setObjectName( tr( "centralWidget" ) );
	setCentralWidget( m_centralWidget );

	m_projectWidget = new ProjectWidget( tr("Project"), this );
	m_projectWidget->setObjectName( tr("Project") );
	addDockWidget(Qt::LeftDockWidgetArea, m_projectWidget);

	m_tilesetWidget = new TilesetWidget( tr( "Tileset" ), this );
	m_tilesetWidget->setObjectName( tr( "Tileset" ) );
	addDockWidget( Qt::LeftDockWidgetArea, m_tilesetWidget );

	m_minimapWidget = new MinimapWidget( tr( "Minimap" ), this );
	m_minimapWidget->setObjectName( tr( "Minimap" ) );
	addDockWidget( Qt::RightDockWidgetArea, m_minimapWidget );

	m_propertiesWidget = new PropertiesWidget( tr( "Properties" ), this );
	m_propertiesWidget->setObjectName( tr( "Properties" ) );
	addDockWidget( Qt::RightDockWidgetArea, m_propertiesWidget );

	m_layerWidget = new LayerWidget( tr( "Layer" ), this );
	m_layerWidget->setObjectName( tr( "Layer" ) );
	addDockWidget( Qt::RightDockWidgetArea, m_layerWidget );

	m_brushWidget = new BrushWidget( tr( "Brush" ), this );
	m_brushWidget->setObjectName( tr( "Layer" ) );
	addDockWidget( Qt::RightDockWidgetArea, m_brushWidget );

	resizeDocks( { m_projectWidget }, { 250 }, Qt::Horizontal);
	resizeDocks( { m_tilesetWidget }, { 250 }, Qt::Vertical );
	resizeDocks( { m_minimapWidget }, { 250 }, Qt::Horizontal );
	resizeDocks( { m_propertiesWidget }, { 250 }, Qt::Vertical );
	resizeDocks( { m_layerWidget }, { 250 }, Qt::Vertical );
	resizeDocks( { m_brushWidget }, { 250 }, Qt::Vertical );

	m_windowsMenu->addAction( m_projectWidget->toggleViewAction() );
	m_windowsMenu->addAction( m_tilesetWidget->toggleViewAction() );
	m_windowsMenu->addAction( m_layerWidget->toggleViewAction() );
	m_windowsMenu->addAction( m_propertiesWidget->toggleViewAction() );
	m_windowsMenu->addAction( m_minimapWidget->toggleViewAction() );
	m_windowsMenu->addAction( m_brushWidget->toggleViewAction() );
}

void MainWindow::initialConnections()
{
	connect( m_eraseSelectedTilesShortcut, &QShortcut::activated, m_centralWidget, &WorkspaceWidget::eraseSelectedTilesInCurrentLayer );
	connect( m_workspaceSwitchTabShortcut, &QShortcut::activated, m_centralWidget, &WorkspaceWidget::nextTab );
	connect( m_workspaceCloseTabShortcut, &QShortcut::activated, m_centralWidget, &WorkspaceWidget::closeCurrentTab );
	connect( m_selectAllTilesShortcut, &QShortcut::activated, m_centralWidget, &WorkspaceWidget::selecteAllTilesInCurrentLayer );
	connect( m_newLayerShortcut, SIGNAL( activated() ), m_layerWidget, SLOT( addNewLayer() ) );
	connect( m_raiseLayerShortcut, &QShortcut::activated, m_layerWidget, &LayerWidget::raiseCurrentLayer );
	connect( m_lowerLayerShortcut, &QShortcut::activated, m_layerWidget, &LayerWidget::lowerCurrentLayer );

	connect( m_saveAction, &QAction::triggered, m_centralWidget, &WorkspaceWidget::saveCurrentMap );
	connect( m_saveAllAction, &QAction::triggered, m_centralWidget, &WorkspaceWidget::saveAllMaps );
	connect( m_mapAction, &QAction::triggered, m_centralWidget, &WorkspaceWidget::addMap );
	connect( m_projectNewAction, &QAction::triggered, m_projectWidget, &ProjectWidget::newProject );
	connect( m_projectOpenAction, SIGNAL( triggered(bool) ), m_projectWidget, SLOT( openProject() ) );
	connect( m_tilesetAction, &QAction::triggered, m_tilesetWidget, &TilesetWidget::addTileset );

	connect( m_centralWidget->m_openProjectButton, SIGNAL( clicked(bool) ), m_projectWidget, SLOT( openProject() ) );
	connect( m_centralWidget->m_newProjectButton, &QPushButton::clicked, m_projectWidget, &ProjectWidget::newProject );

	connect( m_projectWidget, &ProjectWidget::loadProjectSuccessfully, this, &MainWindow::updateToolBar );
	connect( m_projectWidget, &ProjectWidget::loadTilesetSuccessfully, m_tilesetWidget, &TilesetWidget::addTilesetIntoProject );
	connect( m_projectWidget, &ProjectWidget::loadMapSuccessfully, m_centralWidget, &WorkspaceWidget::insertMap );
	connect( m_projectWidget, &ProjectWidget::loadMapSuccessfully, m_layerWidget, &LayerWidget::addNewLayerGroup );
	connect( m_projectWidget, &ProjectWidget::tilesetRenamed, m_tilesetWidget, &TilesetWidget::tilesetRenamed );
	connect( m_projectWidget, &ProjectWidget::isReadyCloseProject, this, &MainWindow::closeCurrentProject );
	connect( m_projectWidget, &ProjectWidget::closeProject, m_centralWidget, &WorkspaceWidget::closeAllTab );
	connect( m_projectWidget, &ProjectWidget::closeProject, m_tilesetWidget, &TilesetWidget::closeAllTab );
	connect( m_projectWidget, &ProjectWidget::loadBrushFile, m_brushWidget, &BrushWidget::addBrush );

	connect( m_centralWidget, &WorkspaceWidget::updateRedo, this, &MainWindow::replaceRedoAction );
	connect( m_centralWidget, &WorkspaceWidget::updateUndo, this, &MainWindow::replaceUndoAction );
	connect( m_centralWidget, &WorkspaceWidget::disableShortcut, this, &MainWindow::disableShortcut );
	connect( m_centralWidget, &WorkspaceWidget::addedNewLayerWithInfo, m_layerWidget, &LayerWidget::implementAddNewLayerWithInfo );
	connect( m_centralWidget, &WorkspaceWidget::getLayerGroupInfoList, m_layerWidget, &LayerWidget::getLayerGroupInfoList );
	connect( m_centralWidget, &WorkspaceWidget::closeTabSuccessfully, m_layerWidget, &LayerWidget::removeLayerGropu );
	connect( m_centralWidget, &WorkspaceWidget::tabFocusChanged, m_layerWidget, &LayerWidget::switchLayerGroup );
	connect( m_centralWidget, &WorkspaceWidget::movedLayerOrder, m_layerWidget, &LayerWidget::moveItem );
	connect( m_centralWidget, &WorkspaceWidget::getLayerIndex, m_layerWidget, &LayerWidget::getLayerIndex );
	connect( m_centralWidget, &WorkspaceWidget::addedNewLayer, m_layerWidget, &LayerWidget::implementAddNewLayer );
	connect( m_centralWidget, &WorkspaceWidget::deletedLayer, m_layerWidget, &LayerWidget::removeLayerFromIndex );
	connect( m_centralWidget, &WorkspaceWidget::renamedLayer, m_layerWidget, &LayerWidget::implementRenameLayer );
	connect( m_centralWidget, &WorkspaceWidget::addNewLayerGroup, m_layerWidget, &LayerWidget::addNewLayerGroup );
	connect( m_centralWidget, &WorkspaceWidget::showProperties, m_propertiesWidget, &PropertiesWidget::showProperties );
	connect( m_centralWidget, &WorkspaceWidget::getPaintMapModified, m_brushWidget, &BrushWidget::getPaintMapModified );

	connect( m_layerWidget, &LayerWidget::addedNewLayerFromIndex,  m_centralWidget, &WorkspaceWidget::addNewLayerIntoMap );
	connect( m_layerWidget, &LayerWidget::modifiedCurrentScene, m_centralWidget, &WorkspaceWidget::markCurrentSceneForModified );
	connect( m_layerWidget, &LayerWidget::setLayerIsVisible,  m_centralWidget, &WorkspaceWidget::setLayerVisible );
	connect( m_layerWidget, &LayerWidget::changeLayerFocus,  m_centralWidget, &WorkspaceWidget::changeLayerFocus );
	connect( m_layerWidget, &LayerWidget::movedLayerGroup,  m_centralWidget, &WorkspaceWidget::changeLayerOrder );
	connect( m_layerWidget, &LayerWidget::setLayerIsLock,  m_centralWidget, &WorkspaceWidget::setLayerLock );
	connect( m_layerWidget, &LayerWidget::setLayerName,  m_centralWidget, &WorkspaceWidget::setLayerName );
	connect( m_layerWidget, &LayerWidget::deletedLayer,  m_centralWidget, &WorkspaceWidget::deleteLayerFromIndex );
	connect( m_layerWidget, &LayerWidget::getTabCount,  m_centralWidget, &WorkspaceWidget::getTabCount );
}

void MainWindow::initialShortcut()
{
	m_saveAction->				setShortcuts( QKeySequence::Save );
	m_undoAction->				setShortcuts( QKeySequence::Undo );
	m_redoAction->				setShortcuts( QKeySequence::Redo );
	m_saveAllAction->			setShortcut( tr( "Ctrl+Shift+S" ) );
	m_cursorToolAction->		setShortcut( tr( "C" ) );
	m_moveToolAction->			setShortcut( tr( "V" ) );
	m_brushAction->				setShortcut( tr( "B" ) );
	m_eraserAction->			setShortcut( tr( "E" ) );
	m_bucketAction->			setShortcut( tr( "G" ) );
	m_magicWandAction->			setShortcut( tr( "W" ) );
	m_selectSameTileAction->	setShortcut( tr( "S" ) );

	m_eraseSelectedTilesShortcut =	new QShortcut( QKeySequence::Delete, this );
	m_selectAllTilesShortcut =		new QShortcut( QKeySequence::SelectAll, this );
	m_workspaceSwitchTabShortcut =	new QShortcut( QKeySequence::NextChild, this );
	m_workspaceCloseTabShortcut =	new QShortcut( tr( "Ctrl+W" ), this );
	m_newLayerShortcut =			new QShortcut( tr( "Ctrl+Shift+N" ), this );
	m_raiseLayerShortcut =			new QShortcut( tr( "Ctrl+[" ), this );
	m_lowerLayerShortcut =			new QShortcut( tr( "Ctrl+]" ), this );
}

void MainWindow::restoreStates()
{
	restoreState( m_config->get( "windowState" ).toByteArray() );
	restoreGeometry( m_config->get( "geometry" ).toByteArray() );
	QString projectFilePath = m_config->get( "projectFilePath" ).toString();
 	m_projectWidget->openProject( projectFilePath );
	QStringList openingMaps = m_config->get( "openingMaps" ).toStringList();
	for ( QString path : openingMaps )
	{
		m_projectWidget->openFile( path );
	}
}

void MainWindow::updateToolBar()
{
	if ( getProject() == nullptr )
	{
		return;
	}

	m_tilesetAction->setDisabled( false );
	m_mapAction->setDisabled( false );
	m_basicToolbar->setDisabled( false );
	m_saveAction->setDisabled( false );
	m_saveAllAction->setDisabled( false );
}

void MainWindow::changeDrawTool( QAction* action )
{
	QVariant var = action->data();
	eDrawTool drawTool = var.value<eDrawTool>();
	m_centralWidget->setDrawTool( drawTool );
}

void MainWindow::replaceRedoAction( QAction* action )
{
	if ( !action )
	{
		action = new QAction( QIcon( ":/MainWindow/Icon/redo.png" ), tr( "&Redo" ) );
		action->setDisabled( true );
	}
	action->setShortcuts( QKeySequence::Redo );
	action->setIcon( QIcon( ":/MainWindow/Icon/redo.png" ) );
	action->setText( tr( "&Redo" ) );
	action->setToolTip( tr( "Redo (Ctrl+Y)" ) );
	m_editMenu->insertAction( m_redoAction, action );
	m_basicToolbar->insertAction( m_redoAction, action );
	m_editMenu->removeAction( m_redoAction );
	m_basicToolbar->removeAction( m_redoAction );
	delete m_redoAction;
	m_redoAction = action;
}

void MainWindow::replaceUndoAction( QAction* action )
{
	if( !action )
	{
		action = new QAction( QIcon( ":/MainWindow/Icon/undo.png" ), tr( "&Undo" ) );
		action->setDisabled( true );
	}
	action->setShortcuts( QKeySequence::Undo );
	action->setIcon( QIcon( ":/MainWindow/Icon/undo.png" ) );
	action->setText( tr( "&Undo" ) );
	action->setToolTip( tr( "Undo (Ctrl+Z)" ) );
	m_editMenu->insertAction( m_undoAction, action );
	m_basicToolbar->insertAction( m_undoAction, action );
	m_editMenu->removeAction( m_undoAction );
	m_basicToolbar->removeAction( m_undoAction );
	delete m_undoAction;
	m_undoAction = action;
}

void MainWindow::disableShortcut( bool isDisable )
{
	Qt::ShortcutContext sc = Qt::WindowShortcut;
	if ( isDisable )
	{
		sc = Qt::WidgetShortcut;
	}
	QList<QAction*> actions = findChildren<QAction*>();
	for( QAction* a : actions ) a->setShortcutContext( sc );

	QList<QShortcut*> shortcuts = findChildren<QShortcut*>();
	for( QShortcut* s : shortcuts )
	{
		s->setEnabled(!isDisable);
		s->setContext( sc );
	}
}

void MainWindow::closeCurrentProject( bool& isSuccess )
{
	isSuccess = true;

	if( !m_centralWidget->isReadyToClose() )
	{
		isSuccess = false;
	}
}
