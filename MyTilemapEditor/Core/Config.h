#pragma once

#include <QVariant>
#include <QSettings>

class Config
{
public:
	Config();
	virtual ~Config();
	void set( const QString& nodeName, const QString& keyName, const QVariant& value );
	void set( const QString& keyName, const QVariant& value );
	QVariant get( const QString& nodeName, const QString& keyName );
	QVariant get( const QString& keyName );

private:
	QString m_fileName;
	QSettings* m_setting;
};
