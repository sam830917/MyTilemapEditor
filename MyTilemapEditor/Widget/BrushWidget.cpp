#include "Widget/BrushWidget.h"
#include "Widget/AddBrushDialog.h"
#include "Brush/Brush.h"
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
	for ( int i = 0; i < m_brushList.size(); ++i )
	{
		delete m_brushList[i];
		m_brushList[i] = nullptr;
	}
}

void BrushWidget::addBrush( Brush* brush )
{
	if ( !brush )
		return;

	m_brushList.push_back(brush);
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
	if ( index < 0 || m_brushList.size() <= index )
	{
		brush = nullptr;
	}
	else
	{
		brush = m_brushList[index];
	}
}

void BrushWidget::createNewBrush()
{
	AddBrushDialog dialog( this );
	QList<BrushType*> brushTypeList = Brush::getAllBrushType();
	int index = m_brushListBox->currentIndex();
 	BrushType* type = brushTypeList[index];
	Brush* newBrush = type->m_constructorFunction();
	dialog.addItem( newBrush->createAddDialogItem() );

	if( dialog.exec() == QDialog::Accepted )
	{
		addBrush( newBrush );
	}
}

void BrushWidget::editBrush( QListWidgetItem* item )
{
	int index = m_listWidget->row( item );
	if ( index <= 0 || index >= m_brushList.size() )
		return;

	AddBrushDialog dialog( this );
	QList<BrushType*> brushTypeList = Brush::getAllBrushType();
	BrushType* type = brushTypeList[m_brushListBox->currentIndex()];
	Brush* newBrush = copyBrush( m_brushList[index], type );
	dialog.addItem( newBrush->createAddDialogItem() );

	if( dialog.exec() == QDialog::Accepted )
	{
		delete m_brushList[index];
		m_brushList[index] = newBrush;
		m_listWidget->item( index )->setText( newBrush->m_name );
	}
	else
	{
		delete newBrush;
	}
}
