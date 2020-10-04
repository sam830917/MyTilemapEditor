#include "WorkspaceWidget.h"
#include "AddMapDialog.h"
#include "Core/Tileset.h"
#include "Utils/ProjectCommon.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QMessageBox>
#include <QFileInfo>
#include <QShortcut>
#include <QSignalMapper>
#include <QDebug>

WorkspaceWidget::WorkspaceWidget( QWidget* parent /*= Q_NULLPTR */ )
	:QWidget(parent)
{
	QVBoxLayout* layoutv = new QVBoxLayout(this);
	QHBoxLayout* layout = new QHBoxLayout(this);

	m_newProjectButton = new QPushButton( this );
	m_newProjectButton->setText(tr("New Project"));
	m_openProjectButton = new QPushButton( this );
	m_openProjectButton->setText( tr( "Open Project" ) );
	layout->addWidget( m_newProjectButton, 0, Qt::AlignLeft | Qt::AlignTop );
	layout->addWidget( m_openProjectButton, 0, Qt::AlignLeft | Qt::AlignTop );
	layout->addStretch();
	layoutv->addLayout(layout);
	setLayout( layoutv );

	QGraphicsView* tilesetView = new QGraphicsView();
	m_mapTabWidget = new QTabWidget( this );
	layoutv->addWidget( m_mapTabWidget );
	m_mapTabWidget->setTabsClosable(true);
	disableTabWidget( true );
	connect(m_mapTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
	connect(m_mapTabWidget, SIGNAL(currentChanged(int)), this, SLOT(changeTab(int)));
	QShortcut* switchShortcut = new QShortcut(QKeySequence::NextChild, this);
	QShortcut* closeShortcut = new QShortcut(tr("Ctrl+W"), this);
	connect(switchShortcut, SIGNAL(activated()), this, SLOT(nextTab()));
	connect(closeShortcut, SIGNAL(activated()), this, SLOT(closeCurrentTab()));
}

void WorkspaceWidget::disableTabWidget( bool disable ) const
{
	if ( disable )
	{
		m_openProjectButton->setVisible( true );
		m_newProjectButton->setVisible( true );
		m_mapTabWidget->setVisible( false );
	}
	else
	{
		m_openProjectButton->setVisible( false );
		m_newProjectButton->setVisible( false );
		m_mapTabWidget->setVisible( true );
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
	m_mapTabWidget->setCurrentIndex( m_mapTabWidget->addTab( mapScene->m_view, mapInfo.getName() ) );

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
	if( !tilesetsEle )
		return;

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
		Layer* newLayer = mapScene->addNewLayer( layerIndex );
		if ( layerInfoList.size() > layerIndex )
		{
			newLayer->setIsLock( layerInfoList[layerIndex].IsLock() );
			newLayer->setIsVisible( layerInfoList[layerIndex].IsVisible() );
		}

		for( XmlElement* tileEle = tilesEle->FirstChildElement( "Tile" ); tileEle; tileEle = tileEle->NextSiblingElement( "Tile" ) )
		{
			int index = parseXmlAttribute( *tileEle, "index", -1 );
			int tilesetNumber = parseXmlAttribute( *tileEle, "tileset", -1 );
			int tilesetIndex = parseXmlAttribute( *tileEle, "tilesetIndex", -1 );
			if( tilesetIndex == -1 || index == -1 || tilesetNumber == -1 )
				return;

			if( tilesetsMap.contains( tilesetNumber ) )
			{
				mapScene->paintMap( index, TileInfo( tilesetsMap.value( tilesetNumber ), tilesetIndex ), layerIndex );
			}
		}
		layerIndex++;
		tilesEle = tilesEle->NextSiblingElement( "Tiles" );
	} while ( tilesEle );

	mapScene->update();
	int currentIndex = 0;
	m_mapSceneList[currentIndex]->m_isSaved = true;
	m_mapTabWidget->setTabText( currentIndex, m_mapSceneList[currentIndex]->m_mapInfo.getName() );
}

void WorkspaceWidget::closeTab( int index )
{
	if ( !m_mapSceneList[index]->m_isSaved )
	{
		QMessageBox msgBox;
		msgBox.setText( "The map has been modified." );
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
}

void WorkspaceWidget::changeTab( int index )
{
	if ( index == -1 )
	{
		updateUndo( nullptr );
		updateRedo( nullptr );
	}
	else
	{
		MapScene* mapScene = m_mapSceneList[index];
		if( mapScene )
		{
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
	getLayerGroupInfoList( tabIndex, layerInfoList );
	for ( int i = 0; i < layerInfoList.size(); ++i )
	{
		XmlElement* mapLayerEle = mapDoc->NewElement( "Layer" );
		mapLayerEle->SetAttribute( "name", layerInfoList[i].getNmae().toStdString().c_str() );
		mapLayerEle->SetAttribute( "isLock", layerInfoList[i].IsLock() );
		mapLayerEle->SetAttribute( "isVisible", layerInfoList[i].IsVisible() );
		mapLayersEle->LinkEndChild( mapLayerEle );
	}

	QMap<QString, int> tilesetsMap;
	// Save map info
	int count = 0;
	MapScene* mapScene = m_mapSceneList[tabIndex];
	for ( int layerIndex = 0; layerIndex < mapScene->m_layers.size(); ++layerIndex )
	{
		XmlElement* mapTiles = mapDoc->NewElement( "Tiles" );
		mapRoot->LinkEndChild( mapTiles );
		Layer* layer = mapScene->m_layers[layerIndex];
		for( int i = 0; i < layer->m_tileList.size(); ++i )
		{
			Tile* tile = layer->m_tileList[i];
			TileInfo tileInfo = tile->getTileInfo();
			if( !tileInfo.isValid() )
				continue;

			if( !tilesetsMap.contains( tileInfo.getTileset()->getRelativeFilePath() ) )
			{
				tilesetsMap.insert( tileInfo.getTileset()->getRelativeFilePath(), count++ );
			}
			int tilesetNumber = tilesetsMap.value( tileInfo.getTileset()->getRelativeFilePath() );
			XmlElement* mapTileEle = mapDoc->NewElement( "Tile" );
			mapTileEle->SetAttribute( "index", i );
			mapTileEle->SetAttribute( "tileset", tilesetNumber );
			mapTileEle->SetAttribute( "tilesetIndex", tileInfo.getIndex() );
			mapTiles->LinkEndChild( mapTileEle );
		}
	}

	// Save tileset
	QMap<QString, int>::const_iterator mapIterator = tilesetsMap.constBegin();
	while( mapIterator != tilesetsMap.constEnd() )
	{
		XmlElement* mapTilesetEle = mapDoc->NewElement( "Tileset" );
		QString path = mapIterator.key();
		mapTilesetEle->SetAttribute( "index", mapIterator.value() );
		mapTilesetEle->SetAttribute( "path", path.toStdString().c_str() );
		mapTilesetsEle->LinkEndChild( mapTilesetEle );
		++mapIterator;
	}

	saveXmlFile( *mapDoc, currentMapScene->getMapInfo().getFilePath() );

	m_mapSceneList[tabIndex]->m_isSaved = true;
	m_mapTabWidget->setTabText( tabIndex, m_mapSceneList[tabIndex]->m_mapInfo.getName() );
}

void WorkspaceWidget::getTabCount( int& tabCount )
{
	tabCount = m_mapTabWidget->count();
}

void WorkspaceWidget::addNewLayerIntoMap( int index )
{
	m_mapSceneList[m_mapTabWidget->currentIndex()]->addNewLayer( index );
}

void WorkspaceWidget::changeLayerOrder( int indexA, int indexB )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	Layer* layerA = mapScene->m_layers[indexA];
	Layer* layerB = mapScene->m_layers[indexB];

	mapScene->m_layers[indexA]->setOrder( indexB );
	mapScene->m_layers[indexB]->setOrder( indexA );
	mapScene->m_layers[indexA] = layerB;
	mapScene->m_layers[indexB] = layerA;

	mapScene->update();
}

void WorkspaceWidget::deleteLayer( int index )
{
	MapScene* mapScene = m_mapSceneList[m_mapTabWidget->currentIndex()];

	Layer* layer = mapScene->m_layers[index];
	mapScene->m_layers.removeAt(index);
	delete layer;
	for ( int i = index; i < mapScene->m_layers.size(); ++i )
	{
		mapScene->m_layers[i]->setOrder( i );
		qDebug() << i;
	}
	mapScene->update();
}

void WorkspaceWidget::lockLayer( int index, bool isLock )
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
