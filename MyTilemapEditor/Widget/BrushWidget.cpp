#include "Widget/BrushWidget.h"
#include "Widget/AddBrushDialog.h"
#include "Brush/BrushParser.h"
#include <QBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QFileInfo>
#include <QPoint>
#include <QDir>
#include <QMessageBox>

BrushWidget::BrushWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_listWidget = new QListWidget( this );
	m_listWidget->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

	QWidget* placeholder = new QWidget( this );
	m_layout = new QBoxLayout( QBoxLayout::BottomToTop, placeholder );
	m_layout->setContentsMargins( 0, 0, 0, 0 );
	initialToolbar();
	m_layout->addWidget( m_listWidget );
	setWidget( placeholder );

	QListWidgetItem* item = new QListWidgetItem( QIcon( ":/MainWindow/Icon/brush_icon.png" ), "Default" );
	m_listWidget->addItem( item );
	m_listWidget->setCurrentRow(0);

	connect( m_listWidget, &QListWidget::itemDoubleClicked, this, &BrushWidget::editBrush );

	m_brushParser = new BrushParser();
	initialBrushFile();
}

BrushWidget::~BrushWidget()
{
	delete m_brushParser;
}

void BrushWidget::addBrush( const QString& filePath )
{
	if ( m_brushParser->loadBrushFile( filePath ) )
	{
		QFileInfo fileInfo( filePath );
		QListWidgetItem* item = new QListWidgetItem( QIcon( ":/MainWindow/Icon/brush_icon.png" ), fileInfo.completeBaseName() );
		m_listWidget->addItem( item );
	}
}

void BrushWidget::initialToolbar()
{
	m_toolbar = new QToolBar;
	m_toolbar->setIconSize( QSize( 20, 20 ) );
	m_layout->addWidget( m_toolbar );

	m_newBrushAction = new QAction( QIcon( ":/MainWindow/Icon/plus.png" ), tr( "&New Brush" ), this );
	m_deleteBrushAction = new QAction( QIcon( ":/MainWindow/Icon/minus.png" ), tr( "&Delete Brush" ), this );
	m_newBrushAction->setToolTip( tr( "New Brush" ) );
	m_deleteBrushAction->setToolTip( tr( "Delete Brush" ) );
	m_brushListBox = new QComboBox( this );
	m_toolbar->addWidget( m_brushListBox );
	m_toolbar->addAction( m_newBrushAction );
	m_toolbar->addAction( m_deleteBrushAction );

 	connect( m_newBrushAction, &QAction::triggered, this, &BrushWidget::createNewBrush );
 	connect( m_deleteBrushAction, &QAction::triggered, this, &BrushWidget::deleteNewBrush );
}

void BrushWidget::initialBrushFile()
{
	QDir brushDirectory( QCoreApplication::applicationDirPath() + "/Brushes" );
	QStringList brushFiles = brushDirectory.entryList( QStringList() << "*.js" << "*.JS", QDir::Files );
	for ( QString fileName : brushFiles )
	{
		QString filePath = brushDirectory.filePath( fileName );
		QFileInfo info( filePath );
		m_brushListBox->addItem( info.completeBaseName(), QVariant::fromValue( filePath ) );
	}
}

void BrushWidget::getPaintMapModified( QList<TileModified>& modifiredList, const QPoint& point, eDrawTool tool )
{
	if ( m_listWidget->currentRow() > 0 )
	{
		modifiredList = m_brushParser->getPaintMapResult( m_listWidget->currentRow() - 1, point, tool );
	}
	else
	{
		modifiredList.push_back( TileModified( point, getCurrentTile() ) );
	}
}

void BrushWidget::createNewBrush()
{
	if ( !getProject() )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "No project is opening!" ) );
		return;
	}

	AddBrushDialog dialog( this );
	QVariant variant = m_brushListBox->currentData();
	QString path = variant.toString();
 	dialog.addItem( m_brushParser->createBrushUI( m_brushListBox->currentText() ) );
	dialog.setFilePath( path );
	dialog.m_brushParser = m_brushParser;

	if( dialog.exec() == QDialog::Accepted )
	{
		QListWidgetItem* item = new QListWidgetItem( QIcon( ":/MainWindow/Icon/brush_icon.png" ), dialog.m_name );
		m_listWidget->addItem( item );
 		saveBrushIntoProject( dialog.m_brushFilePath );
	}
}

void BrushWidget::deleteNewBrush()
{
	if( !getProject() )
	{
		QMessageBox::warning( this, tr( "Warning" ), tr( "No project is opening!" ) );
		return;
	}

	int brushIndex = m_listWidget->currentRow();
	if ( brushIndex == 0 )
	{
		return;
	}
	QMessageBox msgBox;
	msgBox.setWindowTitle( "Confirm Delete Brush" );
	msgBox.setText( "Do you want to delete selected brush?" );
	msgBox.setIcon( QMessageBox::Warning );
	msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
	msgBox.setDefaultButton( QMessageBox::No );
	int ret = msgBox.exec();
	switch( ret )
	{
	case QMessageBox::Yes:
	{
		QString filePath = m_brushParser->getFilePathByIndex( brushIndex - 1 );
		QFile file( filePath );
		file.remove();
		deleteBrushInProject( filePath );
		m_brushParser->deleteBrush( brushIndex - 1 );
		m_listWidget->takeItem( brushIndex );
		break;
	}
	case QMessageBox::No:
		return;
	default:
		// should never be reached
		break;
	}
}

void BrushWidget::closeAllBrush()
{
	int brushCount = m_listWidget->count();
	for ( int i = 1; i < brushCount; ++i )
	{
		delete m_listWidget->takeItem(i);
	}
	m_listWidget->clear();
	QListWidgetItem* item = new QListWidgetItem( QIcon( ":/MainWindow/Icon/brush_icon.png" ), "Default" );
	m_listWidget->addItem( item );
	m_listWidget->setCurrentRow( 0 );
	m_brushParser->deleteAllBrush();
}

void BrushWidget::editBrush( QListWidgetItem* item )
{
	int index = m_listWidget->row( item );
	if ( index <= 0 )
		return;

	AddBrushDialog dialog( this );
	dialog.m_name = m_brushParser->getFileName( index - 1 );
	dialog.m_brushParser = m_brushParser;
	dialog.m_isModify = true;
	dialog.m_brushIndex = index - 1;
	QString brushFilePath = m_brushParser->getBrushFilePathByIndex( index - 1 );
	QString oldFilePath = m_brushParser->getFilePathByIndex( index - 1 );
	dialog.addItem( m_brushParser->createBrushUIByCurrentBrush( index - 1 ) );
	dialog.setFilePath( brushFilePath );

	if( dialog.exec() == QDialog::Accepted )
	{
		m_listWidget->item( index )->setText( dialog.m_name );
		QString newPath = dialog.m_brushFilePath;
		updateBrushFileInProject( oldFilePath, newPath );
	}
}
