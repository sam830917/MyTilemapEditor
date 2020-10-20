#include "Widget/BrushWidget.h"
#include "Widget/AddBrushDialog.h"
#include "Brush/Brush.h"
#include "Utils/ProjectCommon.h"
#include <QBoxLayout>
#include <QToolBar>
#include <QComboBox>

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

	Brush* defaultBrush = new Brush();
	defaultBrush->setName( "Default" );
	addBrush( defaultBrush );
	m_listWidget->setCurrentRow(0);

	connect( m_listWidget, &QListWidget::itemDoubleClicked, this, &BrushWidget::editBrush );
}

BrushWidget::~BrushWidget()
{
}

void BrushWidget::addBrush( Brush* brush, QString filePath )
{
	BrushFile frushFile;
	frushFile.m_brush = brush;
	frushFile.m_filePath = filePath;
	m_brushFileList.push_back(frushFile);
	m_listWidget->addItem( brush->getName() );
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

	QList<BrushType*> brushTypeList = Brush::getAllBrushType();
	for ( BrushType* type : brushTypeList )
	{
		m_brushListBox->addItem( type->m_displayName );
	}
	connect( m_newBrushAction, &QAction::triggered, this, &BrushWidget::createNewBrush );
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

void BrushWidget::createNewBrush()
{
	AddBrushDialog dialog( this );
	QList<BrushType*> brushTypeList = Brush::getAllBrushType();
	int index = m_brushListBox->currentIndex();
 	BrushType* type = brushTypeList[index];
	Brush* newBrush = type->m_constructorFunction();
	newBrush->setBrushType( type );
	dialog.addItem( newBrush->createAddDialogItem() );
	dialog.setBrush( newBrush );

	if( dialog.exec() == QDialog::Accepted )
	{
		addBrush( newBrush, dialog.m_brushFile.m_filePath );
		saveBrushIntoProject( dialog.m_brushFile.m_filePath );
	}
}

void BrushWidget::editBrush( QListWidgetItem* item )
{
	int index = m_listWidget->row( item );
	if ( index <= 0 || index >= m_brushFileList.size() )
		return;

	AddBrushDialog dialog( this );
	QList<BrushType*> brushTypeList = Brush::getAllBrushType();
	BrushType* type = brushTypeList[m_brushListBox->currentIndex()];
	Brush* newBrush = copyBrush( m_brushFileList[index].m_brush );
	dialog.addItem( newBrush->createAddDialogItem() );

	if( dialog.exec() == QDialog::Accepted )
	{
		delete m_brushFileList[index].m_brush;
		m_brushFileList[index].m_brush = newBrush;
		m_listWidget->item( index )->setText( newBrush->getName() );
	}
	else
	{
		delete newBrush;
	}
}
