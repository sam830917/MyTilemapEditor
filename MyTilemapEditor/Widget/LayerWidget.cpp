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

class LayerRowWidget : public QWidget
{
	friend class LayerItem;
	friend class LayerWidget;

public:
	LayerRowWidget( const QString& name, bool lockChecked = false, bool visibleChecked = true, QWidget* parent = Q_NULLPTR );

	QString getName() const;

protected:
	virtual bool eventFilter( QObject* obj, QEvent* event );

private:
	QLineEdit* m_lineEdit;
	QLabel* m_label;
	QStackedWidget* m_stackedWidget;
	QCheckBox* m_lockBtn;
	QCheckBox* m_visibleBtn;
};

LayerWidget::LayerWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_listWidget = new QListWidget();
	m_listWidget->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

	QWidget* placeholder = new QWidget( this );
	m_layout = new QBoxLayout( QBoxLayout::TopToBottom, placeholder );
	m_layout->setContentsMargins( 0, 0, 0, 0 );
	m_layout->addWidget( m_listWidget );
	setWidget( placeholder );

	initialToolbar(); 
	updateToolbarStatus();
	connect( m_listWidget, &QListWidget::itemSelectionChanged, this, &LayerWidget::updateToolbarStatus );
}

void LayerWidget::addNewLayer()
{
	LayerRow* newRow = new LayerRow( m_listWidget );
	if ( m_listWidget->count() == 1 )
	{
		m_listWidget->setCurrentRow( 0 );
	}
	updateToolbarStatus();
}

void LayerWidget::removeLayer()
{
	if ( m_listWidget->count() == 1 )
	{
		return;
	}
	int currentIndex = m_listWidget->currentRow();
	m_listWidget->takeItem( currentIndex );
	updateToolbarStatus();
}

void LayerWidget::raiseCurrentLayer()
{
	int currentRow = m_listWidget->currentRow();
	int targetIndex = currentRow - 1;
	moveItem( currentRow, targetIndex );
}

void LayerWidget::lowerCurrentLayer()
{
	int currentRow = m_listWidget->currentRow();
	int targetIndex = currentRow + 1;
	moveItem( currentRow, targetIndex );
}

void LayerWidget::moveItem( int fromItemIndex, int toItemIndex )
{
	if( toItemIndex < 0 || fromItemIndex < 0 )
	{
		return;
	}

	LayerRow* currentItem = static_cast<LayerRow*>(m_listWidget->item( fromItemIndex ));
	LayerRowWidget* currentW = currentItem->m_layerRow;
	QString name = currentW->getName();
	bool lockChecked = currentW->m_lockBtn->isChecked();
	bool visibleChecked = currentW->m_visibleBtn->isChecked();
	m_listWidget->takeItem( fromItemIndex );
	m_listWidget->insertItem( toItemIndex, currentItem );

	currentItem->m_layerRow = new LayerRowWidget( name, lockChecked, visibleChecked );
	m_listWidget->setItemWidget( m_listWidget->item( toItemIndex ), currentItem->m_layerRow );

	m_listWidget->setCurrentRow( toItemIndex );
	updateToolbarStatus();
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

	connect( m_newLayerAction, &QAction::triggered, this, &LayerWidget::addNewLayer );
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
	int currentRow = m_listWidget->currentRow();

	if ( m_listWidget->count() == 0 )
	{
		updateToolbar( true, false, false, false );
		return;
	}
	if( m_listWidget->count() == 1 )
	{
		updateToolbar( true, false, false, true );
		return;
	}
	if( currentRow <= 0 )
	{
		updateToolbar( true, false, true, true );
	}
	else if( currentRow == m_listWidget->count() - 1 )
	{
		updateToolbar( true, true, false, true );
	}
	else
	{
		updateToolbar( true, true, true, true );
	}
}

LayerRowWidget::LayerRowWidget( const QString& name, bool lockChecked, bool visibleChecked, QWidget* parent /*= Q_NULLPTR */ )
	:QWidget(parent)
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
}

QString LayerRowWidget::getName() const
{
	return m_label->text();
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
			}
		}
		else if( event->type() == QEvent::FocusOut )
		{
			m_label->setText( m_lineEdit->text() );
			m_stackedWidget->setCurrentIndex( 0 );
		}
	}

	return QWidget::eventFilter( obj, event );
}

LayerRow::LayerRow( QListWidget* view )
	:QListWidgetItem()
{
	QString name = QString( "Layer 1" );
	int nameCount = 1;
	int count = view->count();
	for ( int i = 0; i < count; ++i )
	{
		LayerRow* row = static_cast<LayerRow*>(view->item(i));
		if ( name == row->m_layerRow->getName() )
		{
			name = QString( "Layer %1" ).arg( ++nameCount );
			i = -1;
		}
	}

	m_layerRow = new LayerRowWidget( name );
	view->addItem( this );
	view->setItemWidget( this, m_layerRow );
	setSizeHint( QSize( 0, 40 ) );
	view->addItem( this );
}
