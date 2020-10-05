#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QMap>

class PropertiesWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit PropertiesWidget( const QString& title, QWidget* parent = Q_NULLPTR );

private:

public slots:
	void showProperties( const QMap<QString, QString>& informationMap );

private:
	QTreeWidget* m_treeWidget;
};