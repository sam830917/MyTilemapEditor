#pragma once

#include <QDockWidget>
#include <QString>
#include <QList>
#include <QQueue>
#include <QModelIndex>
#include "Core/Project.h"
#include "Core/LayerInfo.h"
#include "Core/MapInfo.h"

class Tileset;

QT_FORWARD_DECLARE_CLASS( QFileSystemModel )
QT_FORWARD_DECLARE_CLASS( QStandardItemModel )
QT_FORWARD_DECLARE_CLASS( QTreeView )
QT_FORWARD_DECLARE_CLASS( QVBoxLayout )
QT_FORWARD_DECLARE_CLASS( QMenu )

class ProjectWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit ProjectWidget( const QString &title, QWidget *parent = Q_NULLPTR );

	void addFolderPathToWatingList( const QString& path );

private:
	void initialTreeView();

public slots:
	void newProject();
	void openProject();
	void openProject( const QString& filePath );
	void refresh();
	void addFolder();
	void fileRenamed(const QString &path, const QString &oldName, const QString &newName);
	void openFile( QModelIndex index );
	void openFile( const QString& filePath );

private slots:
	void popupMenu( const QPoint& pos );
	void rename();

signals:
	void isReadyCloseProject( bool& isSuccess );
	void closeProject();
	void loadProjectSuccessfully();
	void loadTilesetSuccessfully( Tileset* tileset );
	void loadMapSuccessfully( MapInfo mapInfo, QList<LayerInfo> layerInfoList );
	void tilesetRenamed( const QString& path, const QString& oldName, const QString& newName );

private:
	QTreeView* m_treeView;
	QFileSystemModel* m_fileModel;
};