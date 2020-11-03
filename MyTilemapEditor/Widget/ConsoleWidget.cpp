#include "Widget/ConsoleWidget.h"
#include <QTextBrowser>
#include <QBoxLayout>

ConsoleWidget* g_consoleWidget = nullptr;

ConsoleWidget::ConsoleWidget( const QString& title, QWidget* parent /*= Q_NULLPTR */ )
	:QDockWidget( title, parent )
{
	m_textBrowser = new QTextBrowser(this);

	QWidget* placeholder = new QWidget( this );
	QBoxLayout* layout = new QBoxLayout( QBoxLayout::TopToBottom, placeholder );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( m_textBrowser );
	setWidget( placeholder );
	g_consoleWidget = this;
}

void ConsoleWidget::print( const QString& text )
{
	m_textBrowser->append( text );
}
