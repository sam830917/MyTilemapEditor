#include "WorkspaceWidget.h"
#include "AddMapDialog.h"
#include "Core/Tileset.h"
#include "Core/UndoCommand.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QMessageBox>
#include <QFileInfo>
#include <QSignalMapper>
#include <QDebug>
#include <QShortcut>
#include <QKeyEvent>
#include <QFileDialog>
#include <QtMath>
#include <QMenu>

MapScene* g_currentMapScene = nullptr;
QList<TileModified> g_copiedTileList;

WorkspaceWidget::WorkspaceWidget( QWidget* parent /*= Q_NULLPTR */ )
	:QWidget(parent)
{
	QVBoxLayout* layoutv = new QVBoxLayout(this);
	QHBoxLayout* layout = new QHBoxLayout(this);

	m_newProjectButton = new QPushButton( this );
	m_newProjectButton->setText(tr("New Project"));
	m_openProjectButton = new QPushButton( this );
	m_openProjectButton->setText( tr( "Open Project" ) );
	m_newMapButton = new QPushButton( this );
	m_newMapButton->setText(tr("New Map"));
	m_newTilesetButton = new QPushButton( this );
	m_newTilesetButton->setText( tr( "New Tileset" ) );
	m_newMapButton->setIcon( QIcon( ":/MainWindow/Icon/map_icon.png" ) );
	m_newTilesetButton->setIcon( QIcon( ":/MainWindow/Icon/tileset_icon.png" ) );

	layout->addWidget( m_newProjectButton, 0, Qt::AlignLeft | Qt::AlignTop );
	layout->addWidget( m_openProjectButton, 0, Qt::AlignLeft | Qt::AlignTop );
	layout->addWidget( m_newMapButton, 0, Qt::AlignLeft | Qt::AlignTop );
	layout->addWidget( m_newTilesetButton, 0, Qt::AlignLeft | Qt::AlignTop );
	layout->addStretch();

	layoutv->addLayout(layout);
	setLayout( layoutv );

	QGraphicsView* tilesetView = new QGraphicsView();
	m_mapTabWidget = new QTabWidget( this );
	layoutv->addWidget( m_mapTabWidget );
	m_mapTabWidget->setTabsClosable(true);
	m_mapTabWidget->installEventFilter( this );
	disableTabWidget( true );
	m_mapTabWidget->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( m_mapTabWidget, &QTabWidget::customContextMenuRequested, this, &WorkspaceWidget::popupRightClickMenu );
	connect( m_mapTabWidget, SIGNAL( tabCloseRequested(int) ), this, SLOT( closeTab( int ) ) );
	connect( m_mapTabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( changeTab( int ) ) );
	connect( m_newMapButton, &QPushButton::clicked, this, &WorkspaceWidget::addMap );
}

void WorkspaceWidget::disableTabWidget( bool disable ) const
{
	if ( disable )
	{
		if ( getProject() )
		{
			m_newMapButton->setVisible( true );
			m_newTilesetButton->setVisible( true );
			m_openProjectButton->setVisible( false );
			m_newProjectButton->setVisible( false );
		}
		else
		{
			m_newMapButton->setVisible( false );
			m_newTilesetButton->setVisible( false );
			m_openProjectButton->setVisible( true );
			m_newProjectButton->setVisible( true );
		}
		m_mapTabWidget->setVisible( false );
	}
	else
	{
		m_newMapButton->setVisible( false );
		m_newTilesetButton->setVisible( false );
		m_openProjectButton->setVisible( false );
		m_newProjectButton->setVisible( false );
		m_mapTabWidget->setVisible( true );
	}
}

bool WorkspaceWidget::eventFilter( QObject* obj, QEvent* event )
{
	if ( m_mapTabWidget == obj )
	{
		if ( event->type() == QEvent::KeyPress )
		{
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if ( keyEvent->key() == Qt::Key::Key_Tab )
			{
				return true;
			}
		}
	}
	return QWidget::eventFilter( obj, event );
}

void WorkspaceWidget::saveLayerToFile( QList<LayerInfo>& layerInfoList, int tabIndex, XmlElement& mapLayersEle, XmlDocument& mapDoc )
{
	getLayerGroupInfoList( tabIndex, layerInfoList );
	for( int i = 0; i < layerInfoList.size(); ++i )
	{
		XmlElement* mapLayerEle = mapDoc.NewElement( "Layer" );
		mapLayerEle->SetAttribute( "name", layerInfoList[i].getNmae().toStdString().c_str() );
		mapLayerEle->SetAttribute( "isLock", layerInfoList[i].isLock() );
		mapLayerEle->SetAttribute( "isVisible", layerInfoList[i].isVisible() );
		if ( layerInfoList[i].getLayerType() == eLayerType::TILE_LAYER )
		{
			mapLayerEle->SetAttribute( "type", "TILE" );
		}
		else if ( layerInfoList[i].getLayerType() == eLayerType::MARKER_LAYER )
		{
			mapLayerEle->SetAttribute( "type", "MARKER" );
			mapLayerEle->SetAttribute( "color", QString("%1,%2,%3").arg(layerInfoList[i].getColor().red())
				.arg(layerInfoList[i].getColor().green()).arg(layerInfoList[i].getColor().blue()).toStdString().c_str() );
		}
		mapLayerEle->SetAttribute( "layerIndex", i );
		mapLayersEle.LinkEndChild( mapLayerEle );
	}
}

void WorkspaceWidget::saveTileToFile( QList<const Tileset*>& tilesetList, int tabIndex, XmlElement& mapRoot, XmlDocument& mapDoc )
{
	MapScene* mapScene = m_mapSceneList[tabIndex];
	for( int layerIndex = 0; layerIndex < mapScene->m_layers.size(); ++layerIndex )
	{
		XmlElement* mapTiles = mapDoc.NewElement( "Tiles" );
 		mapTiles->SetAttribute("layerIndex", layerIndex);
		mapRoot.LinkEndChild( mapTiles );
		Layer* layer = mapScene->m_layers[layerIndex];
		if ( layer->getLayerInfo().getLayerType() == eLayerType::TILE_LAYER )
		{
			TileLayer* tileLayer = dynamic_cast<TileLayer*>(mapScene->m_layers[layerIndex]);
			for( int i = 0; i < tileLayer->m_tileList.size(); ++i )
			{
				Tile* tile = tileLayer->m_tileList[i];
				TileInfo tileInfo = tile->getTileInfo();
				if( !tileInfo.isValid() )
					continue;

				int tilesetNumber = tilesetList.size();
				bool isNewTileset = true;
				for( int tileIndex = 0; tileIndex < tilesetList.size(); ++tileIndex )
				{
					if( *tilesetList[tileIndex] == *tileInfo.getTileset() )
					{
						isNewTileset = false;
						tilesetNumber = tileIndex;
					}
				}
				if( isNewTileset )
				{
					tilesetList.push_back( tileInfo.getTileset() );
				}
				XmlElement* mapTileEle = mapDoc.NewElement( "Tile" );
				mapTileEle->SetAttribute( "index", i );
				mapTileEle->SetAttribute( "tileset", tilesetNumber );
				mapTileEle->SetAttribute( "tilesetIndex", tileInfo.getIndex() );
				mapTiles->LinkEndChild( mapTileEle );
			}
		}
		else if ( layer->getLayerInfo().getLayerType() == eLayerType::MARKER_LAYER )
		{
			MarkerLayer* markerLayer = dynamic_cast<MarkerLayer*>(mapScene->m_layers[layerIndex]);
			for( int i = 0; i < markerLayer->m_tileList.size(); ++i )
			{
				bool isMarked = markerLayer->m_tileList[i]->isMarked();
				if ( !isMarked )
				{
					continue;
				}
				XmlElement* mapTileEle = mapDoc.NewElement( "Tile" );
				mapTileEle->SetAttribute( "index", i );
				mapTiles->LinkEndChild( mapTileEle );
			}
		}
	}
}

void WorkspaceWidget::popupRightClickMenu( const QPoint& pos )
{
	QMenu menu;

	// Draw tools
	menu.addAction( QIcon( ":/MainWindow/Icon/cursor.png" ), tr( "&Cursor (C)" ), [&]() { this->changeDrawTool(eDrawTool::CURSOR); } );
	menu.addAction( QIcon( ":/MainWindow/Icon/magic-wand.png" ), tr( "&Magic Wand (W)" ), [&]() { this->changeDrawTool(eDrawTool::MAGIC_WAND); } );
	menu.addAction( QIcon( ":/MainWindow/Icon/same-tile.png" ), tr( "&Select Same Tile (S)" ), [&]() { this->changeDrawTool(eDrawTool::SELECT_SAME_TILE); } );
	menu.addAction( QIcon( ":/MainWindow/Icon/move.png" ), tr( "&Move (V)" ), [&]() { this->changeDrawTool( eDrawTool::MOVE ); } );
	menu.addSeparator();
	menu.addAction( QIcon( ":/MainWindow/Icon/brush.png" ), tr( "&Brush (B)" ), [&]() { this->changeDrawTool(eDrawTool::BRUSH); } );
	menu.addAction( QIcon( ":/MainWindow/Icon/eraser.png" ), tr( "&Eraser (E)" ), [&]() { this->changeDrawTool(eDrawTool::ERASER); } );
	menu.addAction( QIcon( ":/MainWindow/Icon/bucket.png" ), tr( "&Bucket (G)" ), [&]() { this->changeDrawTool(eDrawTool::BUCKET); } );
	menu.addAction( QIcon( ":/MainWindow/Icon/shape.png" ), tr( "&Shape (U)" ), [&]() { this->changeDrawTool(eDrawTool::SHAPE); } );

	// Redo and undo actions
	int index = m_mapTabWidget->currentIndex();
	QAction* undoAction = nullptr;
	QAction* redoAction = nullptr;
	if ( index != -1 )
	{
		MapScene* mapScene = m_mapSceneList[index];
		if( mapScene )
		{
			menu.addSeparator();
			g_currentMapScene = mapScene;
			undoAction = mapScene->m_undoStack->createUndoAction( this, tr( "&Undo" ) );
			redoAction = mapScene->m_undoStack->createRedoAction( this, tr( "&Redo" ) );
			undoAction->setIcon( QIcon( ":/MainWindow/Icon/undo.png" ) );
			redoAction->setIcon( QIcon( ":/MainWindow/Icon/redo.png" ) );
			menu.addAction( undoAction );
			menu.addAction( redoAction );
		}
	}
	menu.exec( QCursor::pos() );
	undoAction ? delete undoAction : undoAction = nullptr;
	redoAction ? delete redoAction : redoAction = nullptr;
}

void WorkspaceWidget::resetTabWidgetVisibility()
{
	if ( m_mapTabWidget->count() == 0 )
	{
		disableTabWidget( true );
	}
	else
	{
		disableTabWidget( false );
	}
}

void WorkspaceWidget::markCurrentSceneForModified()
{
	int currentIndex = m_mapTabWidget->currentIndex();
	m_mapSceneList[ currentIndex ]->m_isSaved = false;
	m_mapTabWidget->setTabText( currentIndex, "* " + m_mapSceneList[ currentIndex ]->m_mapInfo.getName() );
}

void WorkspaceWidget::addMap()
{
	AddMapDialog dialog( this );
	if( dialog.exec() == QDialog::Accepted )
	{
		MapInfo* mapInfo = dialog.getResult();

		disableTabWidget( false );
		QList<LayerInfo> layerInfoList;
		layerInfoList.push_back( LayerInfo() );
		insertMap( *mapInfo, layerInfoList );
		addNewLayerGroup( *mapInfo, layerInfoList );
	}
}

void WorkspaceWidget::insertMap( MapInfo mapInfo, QList<LayerInfo> layerInfoList )
{
	disableTabWidget( false );
	// check is already exist
	for( int i = 0; i < m_mapSceneList.size(); ++i )
	{
		MapScene* scene = m_mapSceneList[i];
		if ( mapInfo.getFilePath() == scene->getMapInfo().getFilePath() )
		{
			m_mapTabWidget->setCurrentIndex( i );
			return;
		}
	}

	MapScene* mapScene = new MapScene( mapInfo, this );
	m_mapSceneList.push_back( mapScene );
	int tabIndex = m_mapTabWidget->addTab( mapScene->m_view, mapInfo.getName() );
	m_mapTabWidget->setCurrentIndex( tabIndex );
	m_mapTabWidget->setTabIcon( tabIndex, QIcon( ":/MainWindow/Icon/map_icon.png" ) );

	// Set tiles
	XmlDocument* mapDoc = new XmlDocument;
	mapDoc->LoadFile( mapInfo.getFilePath().toStdString().c_str() );
	if( mapDoc->Error() )
	{
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Map File." ) );
		return;
	}

	XmlElement* mapRoot = mapDoc->RootElement();
	XmlElement* tilesetsEle = mapRoot->FirstChildElement( "Tilesets" );
	if( layerInfoList.isEmpty() )
	{
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Map File." ) );
		return;
	}
	if( !tilesetsEle )
	{
		// map is empty
		for ( int i = 0; i < layerInfoList.size(); ++i )
		{
			LayerInfo& info = layerInfoList[i];
			if ( info.getLayerType() == eLayerType::TILE_LAYER )
			{
				TileLayer* newLayer = mapScene->addNewLayer( i );
				newLayer->m_layerInfo = layerInfoList[i];
			}
			else if ( info.getLayerType() == eLayerType::MARKER_LAYER )
			{
				MarkerLayer* newLayer = mapScene->addNewMarkerLayer( i );
				newLayer->m_layerInfo = layerInfoList[i];
			}
		}
		return;
	}

	QMap<int , Tileset*> tilesetsMap;
	for ( XmlElement* tilesetEle = tilesetsEle->FirstChildElement( "Tileset" ); tilesetEle; tilesetEle = tilesetEle->NextSiblingElement( "Tileset" ) )
	{
		int index = parseXmlAttribute( *tilesetEle, "index", -1 );
		if ( index == -1 )
			continue;

		QString relativePath = parseXmlAttribute( *tilesetEle, "path", QString() );
		QString path = getProjectRootPath() + "/" + relativePath;
		QFileInfo fileinfo(path);
		if ( !fileinfo.exists() )
		{
			QMessageBox::warning( this, tr( "Warning" ), tr( "Failed to Load Tileset File." ) + "\n\n" + path );
		}
		else
		{
			tilesetsMap.insert( index, convertToTileset( path ) );
		}
	}

	XmlElement* tilesEle = mapRoot->FirstChildElement( "Tiles" );
	int layerIndex = 0;
	do
	{
		LayerInfo& info = layerInfoList[layerIndex];
		if( info.getLayerType() == eLayerType::TILE_LAYER )
		{
			TileLayer* newLayer = mapScene->addNewLayer( layerIndex );
			if( layerInfoList.size() > layerIndex )
			{
				newLayer->m_layerInfo = layerInfoList[layerIndex];
			}

			QMap<int, TileInfo> tileInfoMap;
			for( XmlElement* tileEle = tilesEle->FirstChildElement( "Tile" ); tileEle; tileEle = tileEle->NextSiblingElement( "Tile" ) )
			{
				int index = parseXmlAttribute( *tileEle, "index", -1 );
				int tilesetNumber = parseXmlAttribute( *tileEle, "tileset", -1 );
				int tilesetIndex = parseXmlAttribute( *tileEle, "tilesetIndex", -1 );
				if( tilesetIndex == -1 || index == -1 || tilesetNumber == -1 )
					continue;;

				if( tilesetsMap.contains( tilesetNumber ) )
				{
					tileInfoMap[index] = TileInfo( tilesetsMap.value( tilesetNumber ), tilesetIndex );
				}
			}
			mapScene->paintMap( tileInfoMap, layerIndex );
		}
		else if( info.getLayerType() == eLayerType::MARKER_LAYER )
		{
			MarkerLayer* newLayer = mapScene->addNewMarkerLayer( layerIndex );
			if( layerInfoList.size() > layerIndex )
			{
				newLayer->m_layerInfo = layerInfoList[layerIndex];
			}

			QMap<int, TileInfo> tileInfoMap;
			for( XmlElement* tileEle = tilesEle->FirstChildElement( "Tile" ); tileEle; tileEle = tileEle->NextSiblingElement( "Tile" ) )
			{
				int index = parseXmlAttribute( *tileEle, "index", -1 );
				if ( index == -1 )
				{
					continue;
				}
				tileInfoMap[index] = TileInfo();
			}
			mapScene->paintMap( tileInfoMap, layerIndex );
		}

		layerIndex++;
		tilesEle = tilesEle->NextSiblingElement( "Tiles" );
	} while ( tilesEle );

	mapScene->update();
}

void WorkspaceWidget::closeTab( int index )
{
	if ( !m_mapSceneList[index]->m_isSaved )
	{
		QMessageBox msgBox;
		msgBox.setText( "The map has been modified." );
		msgBox.setWindowIcon( getApplicationIcon() );
		msgBox.setInformativeText( "Do you want to save your changes?" );
		msgBox.setIcon( QMessageBox::Warning );
		msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
		msgBox.setDefaultButton( QMessageBox::Save );
		int ret = msgBox.exec();
		switch( ret ) 
		{
		case QMessageBox::Save:
			saveMap( index );
			break;
		case QMessageBox::Discard:
			// Don't Save was clicked
			break;
		case QMessageBox::Cancel:
			return;
		default:
			// should never be reached
			break;
		}
	}
	m_mapTabWidget->removeTab( index );
	m_mapSceneList.removeAt( index );
	closeTabSuccessfully( index );
	if( m_mapTabWidget->count() == 0 )
	{
		disableTabWidget( true );
	}
}

void WorkspaceWidget::setDrawTool( eDrawTool drawTool )
{
	m_drawTool = drawTool;
	switch( drawTool )
	{
	case eDrawTool::MOVE:
	{
		QPixmap toolPixmap = QPixmap( ":/MainWindow/Icon/move.png" );
		toolPixmap.setDevicePixelRatio( 2.0 );
		m_currentCursor = QCursor( toolPixmap );
		break;
	}
	case eDrawTool::MAGIC_WAND:
	{
		QPixmap toolPixmap = QPixmap( ":/MainWindow/Icon/magic-wand.png" );
		toolPixmap.setDevicePixelRatio( 2.0 );
		m_currentCursor = QCursor( toolPixmap, 0, 0 );
		break;
	}
	case eDrawTool::BRUSH:
	{
		QPixmap toolPixmap = QPixmap( ":/MainWindow/Icon/brush.png" );
		toolPixmap.setDevicePixelRatio( 2.0 );
		m_currentCursor = QCursor( toolPixmap, 0, 0 );
		break;
	}
	case eDrawTool::ERASER:
	{
		QPixmap toolPixmap = QPixmap( ":/MainWindow/Icon/eraser.png" );
		toolPixmap.setDevicePixelRatio( 2.0 );
		m_currentCursor = QCursor( toolPixmap, 0, 0 );
		break;
	}
	case eDrawTool::BUCKET:
	{
		QPixmap toolPixmap = QPixmap( ":/MainWindow/Icon/bucket.png" );
		toolPixmap.setDevicePixelRatio( 2.0 );
		m_currentCursor = QCursor( toolPixmap, 0, 0 );
		break;
	}
	default:
	{
		m_currentCursor = QCursor( Qt::ArrowCursor );
		break;
	}
	}
	for ( MapScene* scene : m_mapSceneList )
	{
		scene->m_view->updateCursor();
		scene->setIsShowSelection( false );
	}

}

void WorkspaceWidget::changeTab( int index )
{
	if ( index == -1 )
	{
		g_currentMapScene = nullptr;
		updateUndo( nullptr );
		updateRedo( nullptr );
	}
	else
	{
		MapScene* mapScene = m_mapSceneList[index];
		if( mapScene )
		{
			g_currentMapScene = mapScene;
			updateUndo( mapScene->m_undoStack->createUndoAction( this, tr( "&Undo" ) ) );
			updateRedo( mapScene->m_undoStack->createRedoAction( this, tr( "&Redo" ) ) );
		}
	}
	tabFocusChanged( index );
}

void WorkspaceWidget::changeMapScale( const QString& text )
{
	int scale = text.split(" ")[0].toInt();
}

void WorkspaceWidget::nextTab()
{
	int index = m_mapTabWidget->currentIndex();
	if ( index == -1 )
	{
		return;
	}
	m_mapSceneList[index]->setIsShowSelection( false );
	m_mapTabWidget->setCurrentIndex( index+1 == m_mapTabWidget->count() ? 0 : index+1 );
}

void WorkspaceWidget::closeCurrentTab()
{
	int index = m_mapTabWidget->currentIndex();
	if ( index == -1 )
	{
		return;
	}
	closeTab( index );
}

bool WorkspaceWidget::isReadyToClose()
{
	for ( MapScene* scene : m_mapSceneList )
	{
		if ( !scene->m_isSaved )
		{
			QMessageBox msgBox;
			msgBox.setText( "The maps has been modified." );
			msgBox.setWindowIcon( getApplicationIcon() );
			msgBox.setInformativeText( "Do you want to save your changes?" );
			msgBox.setIcon( QMessageBox::Warning );
			msgBox.setStandardButtons( QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel );
			msgBox.setDefaultButton( QMessageBox::SaveAll );
			int ret = msgBox.exec();
			switch( ret ) 
			{
			case QMessageBox::SaveAll:
				saveAllMaps();
				return true;
			case QMessageBox::Discard:
				return true;
			case QMessageBox::Cancel:
				return false;
			default:
				// should never be reached
				break;
			}
		}
	}

	return true;
}

QStringList WorkspaceWidget::getOpeningMapFilePath() const
{
	QStringList filePathList;
	for ( MapScene* scene : m_mapSceneList )
	{
		filePathList << scene->m_mapInfo.getFilePath();
	}
	return filePathList;
}

void WorkspaceWidget::saveCurrentMap()
{
	int mapIndex = m_mapTabWidget->currentIndex();
	if ( mapIndex < 0 )
		return;
	
	saveMap( mapIndex );
}

void WorkspaceWidget::saveAllMaps()
{
	for ( int i = 0; i < m_mapTabWidget->count(); ++i )
	{
		saveMap( i );
	}
}

void WorkspaceWidget::saveMap( int tabIndex )
{
	if ( tabIndex < 0 || m_mapTabWidget->count() <= tabIndex )
		return;

	MapScene* currentMapScene = m_mapSceneList[tabIndex];
	XmlDocument* mapDoc = new XmlDocument;
	mapDoc->LoadFile( currentMapScene->getMapInfo().getFilePath().toStdString().c_str() );
	if( mapDoc->Error() )
		return;

	XmlElement* mapRoot = mapDoc->RootElement();
	mapRoot->DeleteChildren();
	XmlElement* mapTilesetsEle = mapDoc->NewElement( "Tilesets" );
	mapRoot->LinkEndChild( mapTilesetsEle );
	XmlElement* mapLayersEle = mapDoc->NewElement( "Layers" );
	mapRoot->LinkEndChild( mapLayersEle );

	// Save Layer
	QList<LayerInfo> layerInfoList;
	saveLayerToFile( layerInfoList, tabIndex, *mapLayersEle, *mapDoc );

	// Save map info
	QList<const Tileset*> tilesetList;
	saveTileToFile( tilesetList, tabIndex, *mapRoot, *mapDoc );

	// Save tileset
	for ( int i = 0; i < tilesetList.size(); ++i )
	{
		XmlElement* mapTilesetEle = mapDoc->NewElement( "Tileset" );
		const Tileset* tileset = tilesetList[i];
		QString path = tileset->getRelativeFilePath();
		mapTilesetEle->SetAttribute( "index", i );
		mapTilesetEle->SetAttribute( "path", path.toStdString().c_str() );
		mapTilesetsEle->LinkEndChild( mapTilesetEle );
	}

	saveXmlFile( *mapDoc, currentMapScene->getMapInfo().getFilePath() );

	m_mapSceneList[tabIndex]->m_isSaved = true;
	m_mapTabWidget->setTabText( tabIndex, m_mapSceneList[tabIndex]->m_mapInfo.getName() );
}

void WorkspaceWidget::getTabCount( int& tabCount )
{
	tabCount = m_mapTabWidget->count();
}

void WorkspaceWidget::addNewLayerIntoMap( int index, const QString& name )
{
	LayerAddCommand* command = new LayerAddCommand( m_mapSceneList[m_mapTabWidget->currentIndex()], index, name, eLayerType::TILE_LAYER );
	m_mapSceneList[m_mapTabWidget->currentIndex()]->m_undoStack->push( command );
}

void WorkspaceWidget::addNewMarkerLayerIntoMap( int index, const QString& name )
{
	LayerAddCommand* command = new LayerAddCommand( m_mapSceneList[m_mapTabWidget->currentIndex()], index, name, eLayerType::MARKER_LAYER );
	m_mapSceneList[m_mapTabWidget->currentIndex()]->m_undoStack->push( command );
}

void WorkspaceWidget::changeLayerOrder( int indexA, int indexB )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	LayerMoveCommand* command = new LayerMoveCommand( mapScene, indexA, indexB );
	mapScene->m_undoStack->push( command );
}

void WorkspaceWidget::deleteLayerFromIndex( int index )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	LayerDeleteCommand* command = new LayerDeleteCommand( mapScene, index );
	mapScene->m_undoStack->push( command );
}

void WorkspaceWidget::setLayerLock( int index, bool isLock )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	mapScene->m_layers[index]->setIsLock( isLock );
}

void WorkspaceWidget::setLayerVisible( int index, bool isVisible )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	mapScene->m_layers[index]->setIsVisible( isVisible );
	m_mapSceneList[m_mapTabWidget->currentIndex()]->update();
}

void WorkspaceWidget::setLayerColor( int index, const QColor& color )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	LayerColorChangeCommand* command = new LayerColorChangeCommand( mapScene, index, color );
	mapScene->m_undoStack->push( command );
	m_mapSceneList[m_mapTabWidget->currentIndex()]->update();
}

void WorkspaceWidget::setLayerName( int index, const QString& name )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];
	if ( mapScene->m_layers[index]->getLayerInfo().getNmae() == name )
	{
		return;
	}

	LayerRenameCommand* command = new LayerRenameCommand( mapScene, index, name );
	mapScene->m_undoStack->push( command );
}

void WorkspaceWidget::changeLayerFocus()
{
	m_mapSceneList[m_mapTabWidget->currentIndex()]->showSelectedTileProperties();
}

void WorkspaceWidget::eraseSelectedTilesInCurrentLayer()
{
	m_mapSceneList[m_mapTabWidget->currentIndex()]->eraseSelectedTiles();
}

void WorkspaceWidget::selecteAllTilesInCurrentLayer()
{
	m_mapSceneList[m_mapTabWidget->currentIndex()]->selecteAllTiles();
}

void WorkspaceWidget::closeAllTab()
{
	int count = m_mapTabWidget->count();
	for( int i = 0; i < count; ++i ) 
	{
		closeTab( m_mapTabWidget->currentIndex() );
	}
	m_mapTabWidget->clear();
	disableTabWidget( true );

	for ( int i = 0; i < m_mapSceneList.size(); ++i )
	{
		delete m_mapSceneList[i];
	}
	m_mapSceneList.clear();
	g_copiedTileList.clear();
}

void WorkspaceWidget::exportXMLFile()
{
	if ( !getProject() )
	{
		// warning
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a XML File." ) );
		return;
	}
	if ( m_mapSceneList.empty() )
	{
		// warning
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a XML File." ) );
		return;
	}
	int tabIndex = m_mapTabWidget->currentIndex();
	QString mapName = m_mapSceneList[tabIndex]->m_mapInfo.getName();
	QFileDialog dialog( this, "Export", mapName );
	dialog.setFileMode( QFileDialog::AnyFile );
	dialog.setAcceptMode( QFileDialog::AcceptMode::AcceptSave );
	dialog.setDefaultSuffix( ".xml" );
	QStringList filePathList;
	if( dialog.exec() == QDialog::Accepted )
	{
		filePathList = dialog.selectedFiles();
		if( filePathList.empty() )
		{
			// warning
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a XML File." ) );
			return;
		}
		QString filePath = filePathList[0];
		if( QFileInfo( filePath ).suffix() != "xml" )
		{
			// warning
 			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a XML File." ) );
			return;
		}

		MapScene* currentMapScene = m_mapSceneList[tabIndex];
		XmlDocument* mapDoc = new XmlDocument;
		mapDoc->LoadFile( currentMapScene->getMapInfo().getFilePath().toStdString().c_str() );
		if( mapDoc->Error() )
			return;

		XmlElement* mapRoot = mapDoc->RootElement();
		mapRoot->DeleteChildren();
		XmlElement* mapTilesetsEle = mapDoc->NewElement( "Tilesets" );
		mapRoot->LinkEndChild( mapTilesetsEle );
		XmlElement* mapLayersEle = mapDoc->NewElement( "Layers" );
		mapRoot->LinkEndChild( mapLayersEle );

		// Save Layer
		QList<LayerInfo> layerInfoList;
		saveLayerToFile( layerInfoList, tabIndex, *mapLayersEle, *mapDoc );

		// Save map info
		QList<const Tileset*> tilesetList;
		saveTileToFile( tilesetList, tabIndex, *mapRoot, *mapDoc );

		// Save tileset
		for( int i = 0; i < tilesetList.size(); ++i )
		{
			const Tileset* tileset = tilesetList[i];
			XmlElement* mapTilesetEle = mapDoc->NewElement( "Tileset" );
			XmlElement* mapTilesetImageEle = mapDoc->NewElement( "Image" );
			mapTilesetEle->SetAttribute( "index", i );
			mapTilesetImageEle->SetAttribute( "path", tileset->getImagePath().toStdString().c_str() );
			QString size = QString("%1,%2").arg( tileset->getTileSize().width() ).arg( tileset->getTileSize().height() );
			mapTilesetImageEle->SetAttribute( "size", size.toStdString().c_str() );
			mapTilesetEle->LinkEndChild( mapTilesetImageEle );
			mapTilesetsEle->LinkEndChild( mapTilesetEle );
		}

		saveXmlFile( *mapDoc, filePath );
		QMessageBox msgBox;
		msgBox.setWindowIcon( getApplicationIcon() );
		msgBox.setText( "Map has exported successfully." );
		msgBox.exec();
	}
}

void WorkspaceWidget::exportPNGFile()
{
	if( !getProject() )
	{
		// warning
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a PNG File." ) );
		return;
	}
	if( m_mapSceneList.empty() )
	{
		// warning
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a PNG File." ) );
		return;
	}
	int tabIndex = m_mapTabWidget->currentIndex();
	QString mapName = m_mapSceneList[tabIndex]->m_mapInfo.getName();
	QFileDialog dialog( this, "Export", mapName );
	dialog.setFileMode( QFileDialog::AnyFile );
	dialog.setAcceptMode( QFileDialog::AcceptMode::AcceptSave );
	dialog.setDefaultSuffix( ".png" );
	QStringList filePathList;
	if( dialog.exec() == QDialog::Accepted )
	{
		filePathList = dialog.selectedFiles();
		if( filePathList.empty() )
		{
			// warning
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a PNG File." ) );
			return;
		}
		QString filePath = filePathList[0];
		if( QFileInfo( filePath ).suffix() != "png" )
		{
			// warning
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to create a PNG File." ) );
			return;
		}

		MapScene* currentMapScene = m_mapSceneList[tabIndex];
		MapScene* cloneScene = new MapScene( currentMapScene->m_mapInfo );
		QSize& mapTileSize = currentMapScene->getMapInfo().getTileSize();
		QStyleOptionGraphicsItem opt;
		for ( int i = 0; i < currentMapScene->m_layers.size(); ++i )
		{
			Layer* layer = currentMapScene->m_layers[i];
			if ( layer->getLayerInfo().getLayerType() != eLayerType::TILE_LAYER )
			{
				continue;
			}
			TileLayer* tileLayer = dynamic_cast<TileLayer*>(layer);
			TileLayer* cloneLayer = cloneScene->addNewLayer( tileLayer->getOrder() );
			
			for ( int tileIndex = 0; tileIndex < tileLayer->m_tileList.size(); ++tileIndex )
			{
				cloneLayer->m_tileList[tileIndex]->m_tileInfo = tileLayer->m_tileList[tileIndex]->m_tileInfo;
				cloneLayer->m_tileList[tileIndex]->update();
			}
		}
		QImage image( cloneScene->sceneRect().size().toSize(), QImage::Format_ARGB32 );
		image.fill( Qt::transparent );

		QPainter painter( &image );
		cloneScene->render( &painter );
		image.save( filePath );
		delete cloneScene;
		cloneScene = Q_NULLPTR;
		QMessageBox msgBox;
		msgBox.setWindowIcon( getApplicationIcon() );
		msgBox.setText( "Image has exported successfully." );
		msgBox.exec();
	}
}

void WorkspaceWidget::copySelectedTile()
{
	QList<TileModified> tileModifiedList = m_mapSceneList[m_mapTabWidget->currentIndex()]->getCopiedTiles();
	g_copiedTileList = tileModifiedList;
}

void WorkspaceWidget::cutSelectedTile()
{
	copySelectedTile();
	eraseSelectedTilesInCurrentLayer();
}

void WorkspaceWidget::pasteCopiedTile()
{
	if ( g_copiedTileList.empty() )
	{
		return;
	}
	QPoint origin = m_mapSceneList[m_mapTabWidget->currentIndex()]->m_view->mapFromGlobal( QCursor::pos() );
	QPointF mousePos = m_mapSceneList[m_mapTabWidget->currentIndex()]->m_view->mapToScene(origin);
	MapScene* mapScene =  m_mapSceneList[m_mapTabWidget->currentIndex()];
	QPoint coord = QPoint( qFloor( mousePos.x() / mapScene->m_mapInfo.getTileSize().width() ), qFloor( mousePos.y() / mapScene->m_mapInfo.getTileSize().height() ) );
	mapScene->pasteTilesOnCoord( coord, g_copiedTileList );
}
