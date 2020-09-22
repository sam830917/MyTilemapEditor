#include "WorkspaceWidget.h"
#include "AddMapDialog.h"
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
	QGraphicsView* view = new QGraphicsView( this );
	view->setScene( mapScene );
	m_mapTabWidget->addTab( view, mapInfo->getName() );
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
