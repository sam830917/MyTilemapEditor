#include "LayerWidget.h"
#include <QTreeView>
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

LayerWidget::LayerWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_treeWidget = new QTreeWidget();
   	m_treeWidget->setHeaderHidden(true);
 	m_treeWidget->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
	m_treeWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//  	m_treeWidget->header()->setStretchLastSection(false);

	QWidget* placeholder = new QWidget( this );
	m_layout = new QBoxLayout( QBoxLayout::TopToBottom, placeholder );
	m_layout->setContentsMargins( 0, 0, 0, 0 );
	m_layout->addWidget( m_treeWidget );
	setWidget( placeholder );

	LayerItem* item = new LayerItem(m_treeWidget);
	LayerItem* item2 = new LayerItem(m_treeWidget);
	m_treeWidget->addTopLevelItem( item );
	m_treeWidget->addTopLevelItem( item2 );

	initialToolbar();
}

void LayerWidget::initialToolbar()
{
	m_toolbar = new QToolBar;
	m_toolbar->setIconSize( QSize( 20,20 ) );
	m_layout->addWidget( m_toolbar );

	QAction* newLayerAction = new QAction( QIcon( ":/MainWindow/Icon/plus.png" ), tr( "&New Layer" ), this );
	m_toolbar->addAction( newLayerAction );
	QAction* raiseAction = new QAction( QIcon( ":/MainWindow/Icon/up-arrow.png" ), tr( "&Raise Layer" ), this );
	m_toolbar->addAction( raiseAction );
	QAction* lowerAction = new QAction( QIcon( ":/MainWindow/Icon/down-arrow.png" ), tr( "&Lower Layer" ), this );
	m_toolbar->addAction( lowerAction );
	QAction* deleteAction = new QAction( QIcon( ":/MainWindow/Icon/delete.png" ), tr( "&Delete Layer" ), this );
	m_toolbar->addAction( deleteAction );

// 	m_toolbar->setDisabled(true);
}

class LayerRowWidget : public QWidget
{
public:
	LayerRowWidget( QWidget* parent = Q_NULLPTR );

protected:
	virtual bool eventFilter( QObject* obj, QEvent* event );

private:
	QLineEdit* m_lineEdit;
	QLabel* m_label;
	QStackedWidget* m_stackedWidget;
};

LayerRowWidget::LayerRowWidget( QWidget* parent /*= Q_NULLPTR */ )
	:QWidget(parent)
{
	m_stackedWidget = new QStackedWidget( this );
	m_lineEdit = new QLineEdit( m_stackedWidget );
	m_lineEdit->setText( "Layer" );
	m_label = new QLabel( "Layer", m_stackedWidget );
	m_stackedWidget->addWidget( m_label );
	m_stackedWidget->addWidget( m_lineEdit );
	m_label->installEventFilter( this );
	m_lineEdit->installEventFilter( this );

	QHBoxLayout* h = new QHBoxLayout( this );
	QCheckBox* lockBtn = new QCheckBox;
	QCheckBox* visibleBtn = new QCheckBox;
	h->addWidget( m_stackedWidget );
	h->addWidget( lockBtn );
	h->addWidget( visibleBtn );
	visibleBtn->setChecked( true );

	QString lockStyle = QString( "QCheckBox::indicator:unchecked{image:url(%1);}"
		"QCheckBox::indicator:checked{image:url(%2);}"
	).arg( ":/MainWindow/Icon/unlock.png", ":/MainWindow/Icon/lock.png" );
	lockBtn->setStyleSheet( lockStyle );

	QString visibleStyle = QString( "QCheckBox::indicator:unchecked{image:url(%1);}"
		"QCheckBox::indicator:checked{image:url(%2);}"
	).arg( ":/MainWindow/Icon/close-eye.png", ":/MainWindow/Icon/eye.png" );
	visibleBtn->setStyleSheet( visibleStyle );
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

LayerItem::LayerItem( QTreeWidget* view )
	:QTreeWidgetItem(view)
{
	LayerRowWidget* layerRow = new LayerRowWidget();
	view->setItemWidget( this, 0, layerRow );
}
