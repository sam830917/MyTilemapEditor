#pragma once

#include <QString>
#include <QStringList>
#include "Utils/XmlUtils.h"

class Project
{
public:
	Project( XmlDocument* projectXml, const QString& filePath );
	~Project();

	QStringList		getDirectories();
	XmlDocument*	getDocument() const { return m_projectXml; }
	QString			getFilePath() const { return m_filePath; }
	QString			getName() const;

	void			renameTilesetFile( const QString& oldNamePath, const QString& newNamePath );

private:
	XmlDocument* m_projectXml = nullptr;
	QString m_filePath;
};