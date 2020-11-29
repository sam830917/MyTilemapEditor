#include "ProjectWidget.h"
#include "Core/MapInfo.h"
#include "Utils/XmlUtils.h"
#include "Utils/ProjectCommon.h"
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

class MyFileSystemModel : public QFileSystemModel
{
public:
	MyFileSystemModel(QObject *parent = Q_NULLPTR);
protected:
	QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
};

MyFileSystemModel::MyFileSystemModel( QObject* parent /*= Q_NULLPTR*/ )
	:QFileSystemModel(parent)
{
}

QVariant MyFileSystemModel::data( const QModelIndex& index, int role ) const
{
	if( role == Qt::DecorationRole )
	{
		QFileInfo info = MyFileSystemModel::fileInfo( index );
		if( info.isFile() )
		{
			if( info.suffix() == "map" )
				return QIcon( ":/MainWindow/Icon/map_icon.png" );
			else if( info.suffix() == "tileset" )
				return QIcon( ":/MainWindow/Icon/tileset_icon.png" );
			else if( info.suffix() == "brush" )
				return QIcon( ":/MainWindow/Icon/brush_icon.png" );
		}
	}
	return QFileSystemModel::data( index, role );
}

//----------------------------------------------------------------------------------
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

	openFile( path );
}

void ProjectWidget::openFile( const QString& filePath )
{
	if ( !getProject() )
		return;

	QFileInfo fileInfo( filePath );
	if ( "map" == fileInfo.suffix() )
	{
		MapInfo mapInfo;
		QList<LayerInfo> layerInfoList;
		convertToMapInfo( filePath, mapInfo, layerInfoList );
		if( !mapInfo.IsValid() )
		{
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Map File." ) );
			return;
		}
		loadMapSuccessfully( mapInfo, layerInfoList );
	}
	else if ( "tileset" == fileInfo.suffix() )
	{
		Tileset* tileset  = convertToTileset( filePath );
		if ( tileset == nullptr )
		{
			QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Tileset File." ) );
			return;
		}
		loadTilesetSuccessfully( tileset );
	}
	else if( "brush" == fileInfo.suffix() )
	{
		loadBrushFile( filePath );
	}
}

void ProjectWidget::initialTreeView()
{
	m_fileModel = new MyFileSystemModel( this );
	QStringList nameFilter;
	nameFilter << "*.tileset" << "*.map" << "*.brush";
	m_fileModel->setNameFilters( nameFilter );
	m_fileModel->setNameFilterDisables( false );
	connect( m_fileModel, &QFileSystemModel::fileRenamed, this, &ProjectWidget::fileRenamed );

	m_treeView = new QTreeView( this );
	setWidget( m_treeView );
	m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( m_treeView, &QTreeView::customContextMenuRequested, this, &ProjectWidget::popupMenu );
	connect( m_treeView, SIGNAL( doubleClicked(QModelIndex) ), this, SLOT( openFile(QModelIndex) ) );
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

			if( getProject() )
			{
				// Close previous project
				bool isSuccess = false;
				isReadyCloseProject( isSuccess );
				if( !isSuccess )
				{
					return;
				}
				closeProject();
				updateProject( nullptr );
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
			m_treeView->setSortingEnabled( true );
			m_treeView->sortByColumn( 2, Qt::AscendingOrder );
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
		openProject( filePath );
	}
}

void ProjectWidget::openProject( const QString& filePath )
{
	if ( filePath.isEmpty() )
	{
		return;
	}
	if( QFileInfo( filePath ).suffix() != "mytilemap-project" )
	{
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Project File." ) );
		return;
	}
	QFileInfo fileInfo( filePath );

	if( !fileInfo.exists() )
		return;

	XmlDocument* xmlDocument = new XmlDocument;
	xmlDocument->LoadFile( filePath.toStdString().c_str() );
	if( xmlDocument->Error() )
	{
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Project File." ) );
		return;
	}

	if ( getProject() )
	{
		// Close previous project
		bool isSuccess = false;
		isReadyCloseProject( isSuccess );
		if ( !isSuccess )
		{
			return;
		}
		closeProject();
		updateProject( nullptr );
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
	m_treeView->setSortingEnabled( true );
	m_treeView->sortByColumn( 2, Qt::AscendingOrder );

	// Load Project
	XmlElement* root = xmlDocument->RootElement();
	if( !root )
	{
		QMessageBox::critical( this, tr( "Error" ), tr( "Failed to Load Project File." ) );
		return;
	}
	loadProjectSuccessfully();

	// Load Tileset
	XmlElement* tilesetsEle = root->FirstChildElement( "Tilesets" );
	if( tilesetsEle )
	{
		for( XmlElement* tilesetEle = tilesetsEle->FirstChildElement( "Tileset" ); tilesetEle; tilesetEle = tilesetEle->NextSiblingElement( "Tileset" ) )
		{
			QString path = parseXmlAttribute( *tilesetEle, "path", QString() );
			QString tilesetPath = getProjectRootPath() + "/" + path;

			Tileset* t = convertToTileset( tilesetPath );
			if( t )
			{
				loadTilesetSuccessfully( t );
			}
		}
	}

	// Load Brush
	XmlElement* brushesEle = root->FirstChildElement( "Brushes" );
	if( brushesEle )
	{
		for( XmlElement* brushEle = brushesEle->FirstChildElement( "Brush" ); brushEle; brushEle = brushEle->NextSiblingElement( "Brush" ) )
		{
			QString path = parseXmlAttribute( *brushEle, "path", QString() );
			QString brushPath = getProjectRootPath() + "/" + path;

			loadBrushFile( brushPath );
		}
	}
}

void ProjectWidget::popupMenu( const QPoint& pos )
{
	QMenu menu;
	QModelIndex curIndex = m_treeView->indexAt( pos );
	if ( curIndex.isValid() )
	{
		//menu.addAction( "Rename", this, &ProjectWidget::rename );
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

void ProjectWidget::rename()
{
	QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
	QFileInfo fileInfo = m_fileModel->fileInfo( m_treeView->currentIndex() );
	QString path = fileInfo.absoluteFilePath();
}
