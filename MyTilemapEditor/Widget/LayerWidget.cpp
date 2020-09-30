#include "LayerWidget.h"
#include <QTreeView>
#include <QMenuBar>
#include <QBoxLayout>
#include <QToolBar>

LayerWidget::LayerWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_treeWidget = new QTreeWidget();

	QWidget* placeholder = new QWidget();
	m_layout = new QBoxLayout( QBoxLayout::TopToBottom, placeholder );
	m_layout->setContentsMargins( 0, 0, 0, 0 );
	m_layout->addWidget( m_treeWidget );
	setWidget( placeholder );

	initialToolbar();
}

void LayerWidget::initialToolbar()
{
	QToolBar* toolbar = new QToolBar;
	toolbar->setIconSize( QSize( 20,20 ) );
	m_layout->addWidget( toolbar );

	QAction* newLayerAction = new QAction( QIcon( ":/MainWindow/Icon/plus.png" ), tr( "&New Layer" ), this );
	toolbar->addAction( newLayerAction );
	QAction* raiseAction = new QAction( QIcon( ":/MainWindow/Icon/up-arrow.png" ), tr( "&Raise Layer" ), this );
	toolbar->addAction( raiseAction );
	QAction* lowerAction = new QAction( QIcon( ":/MainWindow/Icon/down-arrow.png" ), tr( "&Lower Layer" ), this );
	toolbar->addAction( lowerAction );
	QAction* deleteAction = new QAction( QIcon( ":/MainWindow/Icon/delete.png" ), tr( "&Delete Layer" ), this );
	toolbar->addAction( deleteAction );
}
