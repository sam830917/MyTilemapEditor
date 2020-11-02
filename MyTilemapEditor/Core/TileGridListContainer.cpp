#include "Core/TileGridListContainer.h"
#include "Core/TileSelector.h"
#include <QBoxLayout>

TileGridListContainer::TileGridListContainer()
	:ListContainerBase()
{
	m_plusButton = new QPushButton();
	m_plusButton->setIcon( QIcon( ":/MainWindow/Icon/plus.png" ) );
	m_plusButton->setMaximumSize( QSize( 30, 30 ) );
	m_plusButton->setMinimumSize( QSize( 30, 30 ) );

	QObject::connect( m_plusButton, &QPushButton::clicked, [=]()
		{
			addTileSelectorList();
			setExpanded( false );
			setExpanded( true );
		} );
}

TileGridListContainer::~TileGridListContainer()
{

}

void TileGridListContainer::deleteChild( int index )
{
	delete m_tileSelectorList[index];
	m_tileSelectorList.removeAt( index );
	delete m_tileGridSelectorList[index];
	m_tileGridSelectorList.removeAt( index );
	delete m_widget[index];
	m_widget.removeAt( index );
}

void TileGridListContainer::attachedTreeWidget()
{
	for( int i = 0; i < m_initialTileList.size(); ++i )
	{
		addTileSelectorList();
		TileSelector* tileSelector = m_tileSelectorList[i];
		tileSelector->setTileInfo( m_initialTileList[i] );
		TileGridSelector* gridSelector = m_tileGridSelectorList[i];
		gridSelector->setGridState( m_initialStatesList[i] );
	}
	setExpanded( false );
	setExpanded( true );
}

void TileGridListContainer::addTileSelectorList()
{
	QTreeWidgetItem* childItem = new QTreeWidgetItem();
	childItem->setText( 0, QString::number( m_childCount ) );
	setExpanded( true );
	addChild( childItem );

	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	TileGridSelector* gridSelector = new TileGridSelector( m_gridType, QSize( 50, 50 ) );
	m_tileSelectorList.push_back( tileSelector );
	m_tileGridSelectorList.push_back( gridSelector );

	if( treeWidget() )
	{
		QPushButton* removeBtn = new QPushButton;
		removeBtn->setIcon( QIcon( ":/MainWindow/Icon/minus.png" ) );
		removeBtn->setMaximumSize( QSize( 30, 30 ) );
		removeBtn->setMinimumSize( QSize( 30, 30 ) ); 
		QWidget* placeholder = new QWidget();
		m_widget.push_back(placeholder);
		QBoxLayout* layout = new QBoxLayout( QBoxLayout::LeftToRight, placeholder );
		layout->setContentsMargins( 0, 0, 0, 0 );
		layout->addWidget( gridSelector );
		layout->addWidget( tileSelector );
		treeWidget()->setItemWidget( childItem, 0, removeBtn );
		treeWidget()->setItemWidget( childItem, 1, placeholder );

		QObject::connect( removeBtn, &QPushButton::clicked, [=]()
			{
				int index = indexOfChild( childItem );
				removeChild( childItem );
				deleteChild( index );
				setExpanded( false );
				setExpanded( true );
			} );
	}

	m_childCount++;
}
