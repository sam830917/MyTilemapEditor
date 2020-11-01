#include "Widget/BrushWidget.h"
#include "Widget/AddBrushDialog.h"
#include "Brush/Brush.h"
#include "Brush/BrushParser.h"
#include <QBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QFileInfo>
#include <QPoint>
#include <QDir>

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

	m_listWidget->addItem( "Default" );
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
		m_listWidget->addItem( fileInfo.completeBaseName() );
	}
}

void BrushWidget::initialToolbar()
{
	m_toolbar = new QToolBar;
	m_toolbar->setIconSize( QSize( 20, 20 ) );
	m_layout->addWidget( m_toolbar );

	m_newBrushAction = new QAction( QIcon( ":/MainWindow/Icon/plus.png" ), tr( "&New Brush" ), this );
	m_newBrushAction->setToolTip( tr( "New Brush" ) );
	m_brushListBox = new QComboBox( this );
	m_toolbar->addWidget( m_brushListBox );
	m_toolbar->addAction( m_newBrushAction );

// 	QList<BrushType*> brushTypeList = Brush::getAllBrushType();
// 	for ( BrushType* type : brushTypeList )
// 	{
// 		m_brushListBox->addItem( type->m_displayName );
// 	}
 	connect( m_newBrushAction, &QAction::triggered, this, &BrushWidget::createNewBrush );
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

void BrushWidget::getCurrentBrush( Brush*& brush ) const
{
	int index = m_listWidget->currentRow();
	if ( index < 0 || m_brushFileList.size() <= index )
	{
		brush = nullptr;
	}
	else
	{
		brush = m_brushFileList[index].m_brush;
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
	AddBrushDialog dialog( this );

	QVariant variant = m_brushListBox->currentData();
	QString path = variant.toString();
 	dialog.addItem( m_brushParser->createBrushUI( m_brushListBox->currentText() ) );
	dialog.setFilePath( path );
	dialog.m_brushParser = m_brushParser;

	if( dialog.exec() == QDialog::Accepted )
	{
		m_listWidget->addItem( dialog.m_name );
 		saveBrushIntoProject( dialog.m_brushFilePath );
	}
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
