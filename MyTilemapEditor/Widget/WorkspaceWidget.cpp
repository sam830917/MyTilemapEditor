#include "WorkspaceWidget.h"
#include "AddMapDialog.h"
#include "../Core/Tileset.h"
#include "../Utils/ProjectCommon.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsView>

WorkspaceWidget::WorkspaceWidget( QWidget* parent /*= Q_NULLPTR */ )
	:QWidget(parent)
{
	QVBoxLayout* layoutv = new QVBoxLayout(this);
	QHBoxLayout* layout = new QHBoxLayout(this);
	//layout->setMargin(10);

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

void WorkspaceWidget::modifiedCurrentScene()
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
		insertMap( mapInfo );
	}
}

void WorkspaceWidget::insertMap( MapInfo* mapInfo )
{
	disableTabWidget( false );
	// check is already exist
	for( int i = 0; i < m_mapSceneList.size(); ++i )
	{
		MapScene* scene = m_mapSceneList[i];
		if ( mapInfo->getFilePath() == scene->getMapInfo().getFilePath() )
		{
			m_mapTabWidget->setCurrentIndex( i );
			return;
		}
	}

	MapScene* mapScene = new MapScene( *mapInfo, this );
	m_mapSceneList.push_back( mapScene );
	int currentIndex = m_mapTabWidget->addTab( mapScene->m_view, mapInfo->getName() );

	// Set tiles
	XmlDocument* mapDoc = new XmlDocument;
	mapDoc->LoadFile( mapInfo->getFilePath().toStdString().c_str() );
	if( mapDoc->Error() )
		return;

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
		tilesetsMap.insert( index, convertToTileset( path ) );
	}

	XmlElement* tilesEle = mapRoot->FirstChildElement( "Tiles" );
	if ( !tilesEle )
		return;

	for( XmlElement* tileEle = tilesEle->FirstChildElement( "Tile" ); tileEle; tileEle = tileEle->NextSiblingElement( "Tile" ) )
	{
		int index = parseXmlAttribute( *tileEle, "index", -1 );
		int tilesetNumber = parseXmlAttribute( *tileEle, "tileset", -1 );
		int tilesetIndex = parseXmlAttribute( *tileEle, "tilesetIndex", -1 );
		if( tilesetIndex == -1 || index == -1 || tilesetNumber == -1 )
			return;

		mapScene->paintMap( index, TileInfo( tilesetsMap.value( tilesetNumber ), tilesetIndex ) );
	}

	m_mapSceneList[currentIndex]->m_isSaved = true;
	m_mapTabWidget->setTabText( currentIndex, m_mapSceneList[currentIndex]->m_mapInfo.getName() );
}

void WorkspaceWidget::closeTab( int index )
{
	m_mapTabWidget->removeTab(index);
	m_mapSceneList.removeAt(index);
	if ( m_mapTabWidget->count() == 0 )
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

	QMap<QString, int> tilesetsMap;
	// Save map info
	XmlElement* mapTiles = mapDoc->NewElement( "Tiles" );
	mapRoot->LinkEndChild( mapTiles );
	int count = 0;
	for( int i = 0; i < currentMapScene->m_tileList.size(); ++i )
	{
		Tile* tile = currentMapScene->m_tileList[i];
		TileInfo tileInfo = tile->getTileInfo();
		if( !tileInfo.isValid() )
			continue;

		if( !tilesetsMap.contains( tileInfo.getTileset()->getRelativeFilePath() ) )
		{
			tilesetsMap.insert( tileInfo.getTileset()->getRelativeFilePath(), count++ );
		}
		int tilesetNumber = tilesetsMap.value( tileInfo.getTileset()->getFilePath() );
		XmlElement* mapTileEle = mapDoc->NewElement( "Tile" );
		mapTileEle->SetAttribute( "index", i );
		mapTileEle->SetAttribute( "tileset", tilesetNumber );
		mapTileEle->SetAttribute( "tilesetIndex", tileInfo.getIndex() );
		mapTiles->LinkEndChild( mapTileEle );
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
