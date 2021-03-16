#include "BrushUI/TileInfoListContainer.h"
#include "BrushUI/TileSelector.h"
#include <QHBoxLayout>

TileInfoListContainer::TileInfoListContainer()
	:ListContainerBase()
{
	m_plusButton = new QPushButton();
	m_plusButton->setIcon( QIcon( ":/MainWindow/Icon/plus.png" ) );
	m_plusButton->setMaximumSize( QSize( 30, 30 ) );
	m_plusButton->setMinimumSize( QSize( 30, 30 ) );

	QObject::connect( m_plusButton, &QPushButton::clicked, [&]()
		{
			addTileSelectorList();
			setExpanded( false );
			setExpanded( true );
		} 
	);
}

TileInfoListContainer::~TileInfoListContainer()
{
}

void TileInfoListContainer::deleteChild( int index )
{
	delete m_tileSelectorList[index];
	m_tileSelectorList[index] = nullptr;
	m_tileSelectorList.removeAt( index );
}

void TileInfoListContainer::attachedTreeWidget()
{
	for ( int i = 0; i < m_initialTileList.size(); ++i )
	{
		addTileSelectorList();
		TileSelector* tileSelector = m_tileSelectorList[i];
		tileSelector->setTileInfo( m_initialTileList[i] );
	}
	setExpanded( false );
	setExpanded( true );
}

void TileInfoListContainer::addTileSelectorList()
{
	QTreeWidgetItem* childItem = new QTreeWidgetItem(this);
	childItem->setText( 0, QString::number( m_childCount ) );
	setExpanded( true );
	addChild( childItem );

	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	m_tileSelectorList.push_back( tileSelector );

	if( treeWidget() )
	{
		QPushButton* removeBtn = new QPushButton;
		removeBtn->setIcon( QIcon( ":/MainWindow/Icon/minus.png" ) );
		removeBtn->setMaximumSize( QSize( 30, 30 ) );
		removeBtn->setMinimumSize( QSize( 30, 30 ) );
		QWidget* widget = new QWidget();
		QHBoxLayout* layout = new QHBoxLayout();
		layout->addWidget( removeBtn, 0, Qt::AlignRight );
		widget->setLayout( layout );

		treeWidget()->setItemWidget( childItem, 0, widget );
		treeWidget()->setItemWidget( childItem, 1, tileSelector );
		QObject::connect( removeBtn, &QPushButton::clicked, [&]()
			{
				int index = indexOfChild( childItem );
				removeChild( childItem );
				deleteChild( index );
				for ( int i = 0; i < childCount(); ++i )
				{
					QTreeWidgetItem* childItem = child( i );
					childItem->setText( 0, QString::number( i ) );
				}
				setExpanded( false );
				setExpanded( true );
				m_childCount--;
			} );
	}

	m_childCount++;
}
