#include "Widget/BrushWidget.h"
#include "Brush/Brush.h"

static Brush* g_currentBrush;

BrushWidget::BrushWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_listWidget = new QListWidget( this );
	setWidget(m_listWidget);
	m_listWidget->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

	Brush* defaultBrush = new Brush();
	defaultBrush->setName( "Default" );
	addBrush( defaultBrush );
	g_currentBrush = defaultBrush;
	m_listWidget->setCurrentRow(0);
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
