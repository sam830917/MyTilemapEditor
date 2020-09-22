#include "ProjectWidget.h"
#include "../Utils/XmlUtils.h"
#include "../Utils/ProjectCommon.h"
#include <QFileSystemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QStyle>
#include <QStandardItemModel>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

ProjectWidget::ProjectWidget( const QString& title, QWidget* parent )
	:QDockWidget( title, parent )
{
	initialTreeView();
}

void ProjectWidget::refresh()
{
	if ( getProject() == Q_NULLPTR )
		return;


}

void ProjectWidget::addFolder()
{
	if( getProject() == Q_NULLPTR )
		return;

	QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
	QFileInfo fileInfo = m_fileModel->fileInfo( m_treeView->currentIndex() );
	QString path = fileInfo.absoluteFilePath();
	if ( !fileInfo.isDir() )
	{
		path = fileInfo.canonicalPath();
	}
	if( indexes.size() == 0 )
	{
		path = m_fileModel->rootPath();
	}
	QDir dir(path);
	if ( dir.exists() )
	{
		int count = 1;
		QString folderName = "New Folder";
		while ( count <= 100 )
		{
			QString createPath = path + "/" + folderName;
			if ( !QDir(createPath).exists() )
			{
				dir.mkdir( createPath );
				break;
			}
	
			folderName = QString("New Folder(%1)").arg(count);
			count++;
		}
	}
}

void ProjectWidget::fileRenamed( const QString& path, const QString& oldName, const QString& newName )
{
	QString fileSuffix = QFileInfo(oldName).suffix();
	if ( fileSuffix.isEmpty() )
	{
		return;
	}
	// update project file
	getProject()->renameTilesetFile( path + "/" + oldName, path + "/" + newName );
	tilesetRenamed( path, oldName, newName );
}

void ProjectWidget::openFile( QModelIndex index )
{
	QFileInfo fileInfo = m_fileModel->fileInfo( index );
	QString path = fileInfo.absoluteFilePath();
	bool isFile = fileInfo.isFile();
	if ( !isFile )
		return;
	
	if ( "map" == fileInfo.suffix() )
	{
		MapInfo* mapInfo = convertToMapInfo( path );
		if( mapInfo == nullptr )
		{
			return;
		}
		loadMapSuccessfully( mapInfo );
	}
	else if ( "tileset" == fileInfo.suffix() )
	{
		Tileset* tileset  = convertToTileset( path );
		if ( tileset == nullptr )
		{
			return;
		}
		loadTilesetSuccessfully( tileset );
	}
}

void ProjectWidget::initialTreeView()
{
	m_fileModel = new QFileSystemModel( this );
	QStringList nameFilter;
	nameFilter << "*.tileset" << "*.map";
	m_fileModel->setNameFilters( nameFilter );
	m_fileModel->setNameFilterDisables( false );
	//m_fileModel->setReadOnly( false );
	connect( m_fileModel, &QFileSystemModel::fileRenamed, this, &ProjectWidget::fileRenamed );

	m_treeView = new QTreeView( this );
	setWidget( m_treeView );
	m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( m_treeView, &QTreeView::customContextMenuRequested, this, &ProjectWidget::popupMenu );
	connect( m_treeView, &QAbstractItemView::doubleClicked, this, &ProjectWidget::openFile );
}

void ProjectWidget::newProject()
{
	QFileDialog dialog( this, "New Project", "untitledProject" );
	dialog.setFileMode( QFileDialog::AnyFile );
	dialog.setAcceptMode( QFileDialog::AcceptMode::AcceptSave );
	dialog.setDefaultSuffix( ".mytilemap-project" );
	QStringList filePathList;
	if( dialog.exec() == QDialog::Accepted )
	{
		filePathList = dialog.selectedFiles();
		if ( !filePathList.empty() )
		{
			QString filePath = filePathList[0];
			if( QFileInfo( filePath ).suffix() != "mytilemap-project" )
			{
				QMessageBox::critical( this, tr( "Error" ), tr( "Failed to New Project File." ) );
				return;
			}
			QFileInfo fileInfo(filePath);

			QString path = fileInfo.path();
			QString fileName = fileInfo.fileName();

			XmlDocument* xmlDocument = new XmlDocument();
			XmlElement* root = xmlDocument->NewElement( "MyTilemapEditor-Project" );
			xmlDocument->LinkEndChild( root );
			XmlElement* tilesetEle = xmlDocument->NewElement( "Tilesets" );
			root->LinkEndChild( tilesetEle );

			saveXmlFile( *xmlDocument, filePath );
	
			updateProject( new Project( xmlDocument, filePath ) );

			m_fileModel->setRootPath( fileInfo.path() );

			m_treeView->setModel( m_fileModel );
			for( int i = 1; i < m_fileModel->columnCount(); ++i )
			{
				m_treeView->hideColumn( i );
			}
			m_treeView->setHeaderHidden( true );
			m_treeView->setRootIndex( m_fileModel->index( fileInfo.path() ) );
			loadProjectSuccessfully();
		}
	}
}

void ProjectWidget::openProject()
{
	QFileDialog dialog( this, "New Project" );
	dialog.setFileMode( QFileDialog::AnyFile );
	dialog.setDefaultSuffix( ".mytilemap-project" );

	QStringList filePathList;
	if( dialog.exec() == QDialog::Accepted )
	{
		filePathList = dialog.selectedFiles();
		if( filePathList.empty() ) 
			return;

		QString filePath = filePathList[0];
		if ( QFileInfo(filePath).suffix() != "mytilemap-project" )
		{
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Project File." ) );
			return;
		}
		QFileInfo fileInfo( filePath );

		if ( !fileInfo.exists() )
			return;

		XmlDocument* xmlDocument = new XmlDocument;
		xmlDocument->LoadFile( filePath.toStdString().c_str() );
		if( xmlDocument->Error() )
		{
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Project File." ) );
			return;
		}

		updateProject( new Project( xmlDocument, filePath ) );

		m_fileModel->setRootPath( fileInfo.path() );

		m_treeView->setModel( m_fileModel );
		for( int i = 1; i < m_fileModel->columnCount(); ++i )
		{
			m_treeView->hideColumn( i );
		}
		m_treeView->setHeaderHidden( true );
		m_treeView->setRootIndex( m_fileModel->index( fileInfo.path() ) );

		// add Tileset
		XmlElement* root = xmlDocument->RootElement();
		if ( !root )
		{
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Project File." ) );
			return;
		}
		loadProjectSuccessfully();

		XmlElement* tilesetsEle = root->FirstChildElement( "Tilesets" );
		if ( !tilesetsEle )
			return;

		for ( XmlElement* tilesetEle = tilesetsEle->FirstChildElement( "Tileset" ); tilesetEle; tilesetEle = tilesetEle->NextSiblingElement( "Tileset" ) )
		{
			QString path = parseXmlAttribute( *tilesetEle, "path", QString() );
			QString tilesetPath = getProjectRootPath() + "/" + path;

			Tileset* t = convertToTileset( tilesetPath );
			if ( t )
			{
				loadTilesetSuccessfully( t );
			}
		}
	}
}

void ProjectWidget::popupMenu( const QPoint& pos )
{
	QMenu menu;
	QModelIndex curIndex = m_treeView->indexAt( pos );
	if ( curIndex.isValid() )
	{
		menu.addAction( "Add Folder", this, &ProjectWidget::addFolder );
		menu.exec( QCursor::pos() );
	}
	else
	{
		m_treeView->clearSelection();
		menu.addAction( "Add Folder", this, &ProjectWidget::addFolder );
		menu.addAction( "Refresh", this, &ProjectWidget::refresh );
		menu.exec( QCursor::pos() );
	}
}