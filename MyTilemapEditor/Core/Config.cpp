#include "Core/Config.h"
#include <QtCore/QtCore>

Config::Config()
{
	m_fileName = QCoreApplication::applicationDirPath() + "/Config.ini";
	m_setting = new QSettings( m_fileName, QSettings::IniFormat );
}

Config::~Config()
{
	delete m_setting;
	m_setting = 0;
}

void Config::set( const QString& nodeName, const QString& keyName, const QVariant& value )
{
	m_setting->setValue( QString( "/%1/%2" ).arg( nodeName ).arg( keyName ), value );
}

void Config::set( const QString& keyName, const QVariant& value )
{
	m_setting->setValue( keyName, value );
}

QVariant Config::get( const QString& nodeName, const QString& keyName )
{
	QVariant qvar = m_setting->value( QString( "/%1/%2" ).arg( nodeName ).arg( keyName ) );
	return qvar;
}

QVariant Config::get( const QString& keyName )
{
	QVariant qvar = m_setting->value( QString( "/%1" ).arg( keyName ) );
	return qvar;
}
