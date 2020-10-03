#pragma once

#include <QString>

class LayerInfo
{
public:
	LayerInfo( const QString& name = "Layer 1", bool isLock = false, bool isVisible = true );
	~LayerInfo();

	QString		getNmae() const { return m_name; }
	bool		IsLock() const { return m_isLock; }
	bool		IsVisible() const { return m_isVisible; }
private:
	QString m_name;
	bool m_isLock = false;
	bool m_isVisible = true;
};