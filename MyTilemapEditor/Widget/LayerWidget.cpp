#include "LayerWidget.h"
#include <QListWidget>
#include <QMenuBar>
#include <QBoxLayout>
#include <QToolBar>
#include <QCheckBox>
#include <QLineEdit>
#include <QHeaderView>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QVariant>
#include <QPushButton>
#include <QDebug>

LayerGroup::LayerGroup( MapInfo mapInfo, LayerWidget* parent /*= Q_NULLPTR */ )
	:QListWidget(parent),
	m_mapInfo(mapInfo),
	m_layerWidget(parent)
{
}

LayerWidget::LayerWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_emptyListWidget = new QListWidget();
	m_emptyListWidget->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

	QWidget* placeholder = new QWidget( this );
	m_layout = new QBoxLayout( QBoxLayout::BottomToTop, placeholder );
	initialToolbar();
	m_layout->setContentsMargins( 0, 0, 0, 0 );
	m_layout->addWidget( m_emptyListWidget );
	setWidget( placeholder );

	updateToolbarStatus();
}

void LayerWidget::addNewLayer()
{
	QString name = QString( "Layer 1" );
	int nameCount = 1;
	int count = m_listWidgetList[m_currentIndex]->count();
	for( int i = 0; i < count; ++i )
	{
		LayerRow* row = static_cast<LayerRow*>(m_listWidgetList[m_currentIndex]->item( i ));
		if( name == row->m_layerRowWidget->getName() )
		{
			name = QString( "Layer %1" ).arg( ++nameCount );
			i = -1;
		}
	}

	addedNewLayerFromIndex( 0, name );
}

void LayerWidget::addNewLayer( LayerInfo layerInfo )
{
	LayerRow* newRow = new LayerRow( m_listWidgetList[m_currentIndex], layerInfo );
	if( m_listWidgetList[m_currentIndex]->count() == 1 )
	{
		m_listWidgetList[m_currentIndex]->setCurrentRow( 0 );
	}
	updateToolbarStatus();
}

void LayerWidget::implementAddNewLayer( int index, const QString& name )
{
	LayerRow* newRow = new LayerRow( m_listWidgetList[m_currentIndex], index );
	newRow->m_layerRowWidget->m_label->setText(name);
	newRow->m_layerRowWidget->m_lineEdit->setText(name);
	m_listWidgetList[m_currentIndex]->setCurrentRow( index );
	updateToolbarStatus();
	modifiedCurrentScene();
}

void LayerWidget::implementAddNewLayerWithInfo( int index, LayerInfo layerInfo )
{
	LayerRow* newRow = new LayerRow( m_listWidgetList[m_currentIndex], index );
	newRow->setLayerInfo( layerInfo );
	m_listWidgetList[m_currentIndex]->setCurrentRow( index );
	updateToolbarStatus();
	modifiedCurrentScene();
}

void LayerWidget::implementRenameLayer( int index, const QString& name )
{
	LayerRowWidget* layerRow = dynamic_cast<LayerRowWidget*>(m_listWidgetList[m_currentIndex]->itemWidget( m_listWidgetList[m_currentIndex]->item( index ) ));
	layerRow->m_label->setText( name );
	layerRow->m_lineEdit->setText( name );
	modifiedCurrentScene();
}

void LayerWidget::removeLayerFromIndex( int index )
{
	delete m_listWidgetList[m_currentIndex]->takeItem( index );
	updateToolbarStatus();
	modifiedCurrentScene();
}

void LayerWidget::removeLayer()
{
	if ( m_listWidgetList[m_currentIndex]->count() == 1 )
	{
		return;
	}
	int currentIndex = m_listWidgetList[m_currentIndex]->currentRow();
	deletedLayer( currentIndex );
}

void LayerWidget::raiseCurrentLayer()
{
	int currentRow = m_listWidgetList[m_currentIndex]->currentRow();
	int targetIndex = currentRow - 1;
	movedLayerGroup( currentRow, targetIndex );
}

void LayerWidget::lowerCurrentLayer()
{
	int currentRow = m_listWidgetList[m_currentIndex]->currentRow();
	int targetIndex = currentRow + 1;
	movedLayerGroup( currentRow, targetIndex );
}

void LayerWidget::addNewLayerGroup( MapInfo mapInfo, QList<LayerInfo> layerInfoList )
{
	if ( !mapInfo.IsValid() )
	{
		return;
	}
	// check is already exist
	for ( int i = 0; i < m_listWidgetList.size(); ++i )
	{
		if ( m_listWidgetList[i]->m_mapInfo.getFilePath() == mapInfo.getFilePath() )
		{
			switchLayerGroup(i);
			return;
		}
	}

	LayerGroup* listWidget = new LayerGroup(mapInfo, this);
	listWidget->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );
	m_listWidgetList.push_back( listWidget );
	m_layout->addWidget( listWidget );
	connect( listWidget, &QListWidget::itemSelectionChanged, this, &LayerWidget::updateToolbarStatus );
	connect( listWidget, &QListWidget::itemSelectionChanged, this, &LayerWidget::changeLayerFocus );
	m_currentIndex = m_listWidgetList.size() - 1;
	for ( int i = 0; i < layerInfoList.size(); ++i )
	{
		addNewLayer( layerInfoList[i] );
	}

	for ( int i = 0; i < m_listWidgetList.size() - 1; ++i )
	{
		m_listWidgetList[i]->setVisible( false );
	}
	m_emptyListWidget->setVisible( false );
	updateToolbarStatus();
}

void LayerWidget::removeLayerGropu( int listWidgetIndex )
{
	if( m_listWidgetList.size() <= listWidgetIndex )
	{
		return;
	}
	if( listWidgetIndex == -1 )
	{
		return;
	}
	delete m_listWidgetList[listWidgetIndex];
	m_listWidgetList.removeAt( listWidgetIndex );
	if ( m_listWidgetList.size() == 0 )
	{
		m_emptyListWidget->setVisible( true );
	}
	else
	{
		if ( listWidgetIndex == m_listWidgetList.size() )
		{
			switchLayerGroup( listWidgetIndex - 1 );
		}
		else
		{
			switchLayerGroup( listWidgetIndex );
		}
	}
}

void LayerWidget::switchLayerGroup( int listWidgetIndex )
{
	if ( m_listWidgetList.size() <= listWidgetIndex )
	{
		return;
	}
	m_currentIndex = listWidgetIndex;
	if ( listWidgetIndex == -1 )
	{
		return;
	}
	for( int i = 0; i < m_listWidgetList.size(); ++i )
	{
		m_listWidgetList[i]->setVisible( false );
	}
	m_listWidgetList[m_currentIndex]->setVisible( true );
	m_emptyListWidget->setVisible( false );
	updateToolbarStatus();
}

void LayerWidget::getLayerIndex( int& index )
{
	if ( m_currentIndex <= -1 )
	{
		return;
	}
	index = m_listWidgetList[m_currentIndex]->currentRow();
}

void LayerWidget::getLayerGroupInfoList( int index, QList<LayerInfo>& layerInfoList )
{
	LayerGroup* layerGroup = m_listWidgetList[index];
	for ( int i = 0; i < layerGroup->count(); ++i )
	{
		LayerRowWidget* layerRow = dynamic_cast<LayerRowWidget*>( layerGroup->itemWidget( layerGroup->item(i) ) );
		LayerInfo layerInfo = LayerInfo( layerRow->getName(), layerRow->m_lockBtn->isChecked(), layerRow->m_visibleBtn->isChecked() );
		layerInfoList.push_back(layerInfo);
	}
}

void LayerWidget::moveItem( int fromItemIndex, int toItemIndex )
{
	if( toItemIndex < 0 || fromItemIndex < 0 )
	{
		return;
	}

	LayerRow* currentItem = static_cast<LayerRow*>(m_listWidgetList[m_currentIndex]->item( fromItemIndex ));
	LayerRowWidget* currentW = currentItem->m_layerRowWidget;
	QString name = currentW->getName();
	bool lockChecked = currentW->m_lockBtn->isChecked();
	bool visibleChecked = currentW->m_visibleBtn->isChecked();
	m_listWidgetList[m_currentIndex]->takeItem( fromItemIndex );
	m_listWidgetList[m_currentIndex]->insertItem( toItemIndex, currentItem );

	currentItem->m_layerRowWidget = new LayerRowWidget( name, currentItem, lockChecked, visibleChecked );
	m_listWidgetList[m_currentIndex]->setItemWidget( m_listWidgetList[m_currentIndex]->item( toItemIndex ), currentItem->m_layerRowWidget );

	// reorder index
	for ( int i = 0; i < m_listWidgetList[m_currentIndex]->count(); ++i )
	{
		LayerRow* currentItem = static_cast<LayerRow*>(m_listWidgetList[m_currentIndex]->item( i ));
		currentItem->m_index = i;
	}

	m_listWidgetList[m_currentIndex]->setCurrentRow( toItemIndex );
	updateToolbarStatus();
	modifiedCurrentScene();
}

void LayerWidget::initialToolbar()
{
	m_toolbar = new QToolBar;
	m_toolbar->setIconSize( QSize( 20,20 ) );
	m_layout->addWidget( m_toolbar );

	m_newLayerAction = new QAction( QIcon( ":/MainWindow/Icon/plus.png" ), tr( "&New Layer" ), this );
	m_toolbar->addAction( m_newLayerAction );
	m_raiseAction = new QAction( QIcon( ":/MainWindow/Icon/up-arrow.png" ), tr( "&Raise Layer" ), this );
	m_toolbar->addAction( m_raiseAction );
	m_lowerAction = new QAction( QIcon( ":/MainWindow/Icon/down-arrow.png" ), tr( "&Lower Layer" ), this );
	m_toolbar->addAction( m_lowerAction );
	m_deleteAction = new QAction( QIcon( ":/MainWindow/Icon/delete.png" ), tr( "&Delete Layer" ), this );
	m_toolbar->addAction( m_deleteAction );

	connect( m_newLayerAction, SIGNAL( triggered(bool) ), this, SLOT( addNewLayer() ) );
	connect( m_deleteAction, &QAction::triggered, this, &LayerWidget::removeLayer );
	connect( m_raiseAction, &QAction::triggered, this, &LayerWidget::raiseCurrentLayer );
	connect( m_lowerAction, &QAction::triggered, this, &LayerWidget::lowerCurrentLayer );
}

void LayerWidget::updateToolbar( bool enableNewLayer, bool enableRaise, bool enableLower, bool enableDelete )
{
	m_newLayerAction->setDisabled( !enableNewLayer );
	m_raiseAction->setDisabled( !enableRaise );
	m_lowerAction->setDisabled( !enableLower );
	m_deleteAction->setDisabled( !enableDelete );
}

void LayerWidget::updateToolbarStatus()
{
	if ( m_currentIndex == -1 )
	{
		updateToolbar( false, false, false, false );
		return;
	}
	int tabNumber = 0;
	getTabCount( tabNumber );
	if ( tabNumber == 0 )
	{
		updateToolbar( false, false, false, false );
		return;
	}

	int currentRow = m_listWidgetList[m_currentIndex]->currentRow();

	if ( m_listWidgetList[m_currentIndex]->count() == 0 )
	{
		updateToolbar( true, false, false, false );
		return;
	}
	if( m_listWidgetList[m_currentIndex]->count() == 1 )
	{
		updateToolbar( true, false, false, false );
		return;
	}
	if( currentRow <= 0 )
	{
		updateToolbar( true, false, true, true );
	}
	else if( currentRow == m_listWidgetList[m_currentIndex]->count() - 1 )
	{
		updateToolbar( true, true, false, true );
	}
	else
	{
		updateToolbar( true, true, true, true );
	}
}

LayerRowWidget::LayerRowWidget( const QString& name, LayerRow* layerRow, bool lockChecked, bool visibleChecked, QWidget* parent /*= Q_NULLPTR */ )
	:QWidget(parent),
	m_layerRow(layerRow)
{
	m_stackedWidget = new QStackedWidget( this );
	m_lineEdit = new QLineEdit( m_stackedWidget );
	m_lineEdit->setText( name );
	m_label = new QLabel( name, m_stackedWidget );
	m_label->setText( name );
	m_stackedWidget->addWidget( m_label );
	m_stackedWidget->addWidget( m_lineEdit );
	m_label->installEventFilter( this );
	m_lineEdit->installEventFilter( this );

	QHBoxLayout* h = new QHBoxLayout( this );
	m_lockBtn = new QCheckBox;
	m_visibleBtn = new QCheckBox;
	h->addWidget( m_stackedWidget );
	h->addWidget( m_lockBtn );
	h->addWidget( m_visibleBtn );
	m_lockBtn->setChecked( lockChecked );
	m_visibleBtn->setChecked( visibleChecked );

	QString lockStyle = QString( "QCheckBox::indicator:unchecked{image:url(%1);}"
		"QCheckBox::indicator:checked{image:url(%2);}"
	).arg( ":/MainWindow/Icon/unlock.png", ":/MainWindow/Icon/lock.png" );
	m_lockBtn->setStyleSheet( lockStyle );

	QString visibleStyle = QString( "QCheckBox::indicator:unchecked{image:url(%1);}"
		"QCheckBox::indicator:checked{image:url(%2);}"
	).arg( ":/MainWindow/Icon/close-eye.png", ":/MainWindow/Icon/eye.png" );
	m_visibleBtn->setStyleSheet( visibleStyle );
	connect( m_lockBtn, SIGNAL( clicked() ), this, SLOT( setIsLock() ) );
	connect( m_visibleBtn, SIGNAL( clicked() ), this, SLOT( setIsVisible() ) );
}

QString LayerRowWidget::getName() const
{
	return m_label->text();
}

void LayerRowWidget::setIsLock()
{
 	bool checked = m_lockBtn->isChecked();

	m_layerRow->m_layerGroup->setCurrentRow( m_layerRow->m_index );
	m_layerRow->m_layerGroup->m_layerWidget->setLayerIsLock( m_layerRow->m_index, checked );
	m_layerRow->m_layerGroup->m_layerWidget->modifiedCurrentScene();
}

void LayerRowWidget::setIsVisible()
{
	bool checked = m_visibleBtn->isChecked();

	m_layerRow->m_layerGroup->setCurrentRow( m_layerRow->m_index );
	m_layerRow->m_layerGroup->m_layerWidget->setLayerIsVisible( m_layerRow->m_index, checked );
	m_layerRow->m_layerGroup->m_layerWidget->modifiedCurrentScene();
}

bool LayerRowWidget::eventFilter( QObject* obj, QEvent* event )
{
	if( obj == m_label )
	{
		if( event->type() == QEvent::MouseButtonDblClick )
		{
			m_lineEdit->setText( m_label->text() );
			m_lineEdit->setFocus();
			m_stackedWidget->setCurrentIndex( 1 );
		}
	}
	else if( obj == m_lineEdit )
	{
		if( event->type() == QEvent::KeyPress )
		{
			QKeyEvent* keyevt = static_cast<QKeyEvent*>(event);
			if( (keyevt->key() == Qt::Key_Return) ||
				(keyevt->key() == Qt::Key_Escape) ||
				(keyevt->key() == Qt::Key_Enter) )
			{
				m_label->setText( m_lineEdit->text() );
				m_stackedWidget->setCurrentIndex( 0 );

				m_layerRow->setSelected( true );
				QString name = m_label->text();
				m_layerRow->m_layerGroup->m_layerWidget->setLayerName( m_layerRow->m_index, name );
				m_layerRow->m_layerGroup->m_layerWidget->modifiedCurrentScene();
			}
		}
		else if( event->type() == QEvent::FocusOut )
		{
			m_label->setText( m_lineEdit->text() );
			m_stackedWidget->setCurrentIndex( 0 );

			m_layerRow->setSelected( true );
			QString name = m_label->text();
			m_layerRow->m_layerGroup->m_layerWidget->setLayerName( m_layerRow->m_index, name );
			m_layerRow->m_layerGroup->m_layerWidget->modifiedCurrentScene();
		}
	}

	return QWidget::eventFilter( obj, event );
}

LayerRow::LayerRow( LayerGroup* view, int insertIndex )
	:QListWidgetItem(),
	m_layerGroup(view)
{
	LayerInfo info;

	m_layerRowWidget = new LayerRowWidget( info.getNmae(), this );
	m_index = insertIndex;
	view->insertItem( insertIndex, this );
	view->setItemWidget( this, m_layerRowWidget );
	setSizeHint( QSize( 0, 40 ) );
}

LayerRow::LayerRow( LayerGroup* view, LayerInfo layerInfo )
	:QListWidgetItem(),
	m_layerGroup( view )
{
	m_layerRowWidget = new LayerRowWidget( layerInfo.getNmae(), this, layerInfo.isLock(), layerInfo.isVisible() );
	m_index = view->count();
	view->addItem( this );
	view->setItemWidget( this, m_layerRowWidget );
	setSizeHint( QSize( 0, 40 ) );
}

void LayerRow::setLayerInfo( const LayerInfo& layerInfo )
{
	m_layerRowWidget->m_label->setText(layerInfo.getNmae());
	m_layerRowWidget->m_lineEdit->setText(layerInfo.getNmae());
	m_layerRowWidget->m_lockBtn->setChecked(layerInfo.isLock());
	m_layerRowWidget->m_visibleBtn->setChecked(layerInfo.isVisible());
}
