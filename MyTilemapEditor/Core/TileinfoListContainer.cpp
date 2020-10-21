#include "Core/TileInfoListContainer.h"
#include "Core/TileSelector.h"

TileInfoListContainer::TileInfoListContainer()
	:ListContainerBase()
{

}

TileInfoListContainer::TileInfoListContainer( QList<TileInfo>* connectList )
	: ListContainerBase( false ),
	m_connectList(connectList)
{
	m_plusButton = new QPushButton();
	m_plusButton->setIcon( QIcon( ":/MainWindow/Icon/plus.png" ) );
	m_plusButton->setMaximumSize( QSize( 30, 30 ) );
	m_plusButton->setMinimumSize( QSize( 30, 30 ) );

	for ( int i = 0; i < connectList->size(); ++i )
	{
		addTileSelectorList();
	}

	QObject::connect( m_plusButton, &QPushButton::clicked, [=]()
		{
			addTileSelectorList();
			setExpanded( false );
			setExpanded( true );
		} );
}

TileInfoListContainer::~TileInfoListContainer()
{
}

void TileInfoListContainer::deleteChild( int index )
{
	delete m_tileSelectorList[index];
	m_tileSelectorList.removeAt( index );
}

void TileInfoListContainer::attachedTreeWidget()
{
	for ( int i = 0; i < childCount(); ++i )
	{
		QTreeWidgetItem* childItem = child(i);

		QPushButton* removeBtn = new QPushButton;
		removeBtn->setIcon( QIcon( ":/MainWindow/Icon/minus.png" ) );
		removeBtn->setMaximumSize( QSize( 30, 30 ) );
		removeBtn->setMinimumSize( QSize( 30, 30 ) );
		treeWidget()->setItemWidget( childItem, 0, removeBtn );
		treeWidget()->setItemWidget( childItem, 1, m_tileSelectorList[i] );

		QObject::connect( removeBtn, &QPushButton::clicked, [=]()
			{
				int index = indexOfChild( childItem );
				removeChild( childItem );
				deleteChild( index );
				m_connectList->removeAt( index );
				setExpanded(false);
				setExpanded(true);
			} );
	}
}

void TileInfoListContainer::addTileSelectorList()
{
	QTreeWidgetItem* childItem = new QTreeWidgetItem();

	childItem->setText( 0, QString::number( m_childCount ) );
	setExpanded( true );
	addChild( childItem );
	int childIndex = indexOfChild( childItem );

	TileSelector* tileSelector = new TileSelector( QSize( 50, 50 ) );
	TileInfo newTileinfo;
	if( m_connectList->size() > childIndex )
	{
		tileSelector->setTileInfo( TileInfo( m_connectList->at( childIndex ).getTileset(), m_connectList->at( childIndex ).getIndex() ) );
	}
	else
	{
		m_connectList->push_back( newTileinfo );
	}
	m_tileSelectorList.push_back( tileSelector );
	QObject::connect( tileSelector, &TileSelector::tileChanged, [=]( TileInfo tileInfo )
		{
			m_connectList->removeAt( childIndex );
			m_connectList->insert( childIndex, tileInfo );
		}
	);

	if( treeWidget() )
	{
		QPushButton* removeBtn = new QPushButton;
		removeBtn->setIcon( QIcon( ":/MainWindow/Icon/minus.png" ) );
		removeBtn->setMaximumSize( QSize( 30, 30 ) );
		removeBtn->setMinimumSize( QSize( 30, 30 ) );
		treeWidget()->setItemWidget( childItem, 0, removeBtn );
		treeWidget()->setItemWidget( childItem, 1, tileSelector );

		QObject::connect( removeBtn, &QPushButton::clicked, [=]()
			{
				int index = indexOfChild( childItem );
				removeChild( childItem );
				deleteChild( index );
				m_connectList->removeAt( index );
				setExpanded( false );
				setExpanded( true );
			} );
	}

	m_childCount++;
}
