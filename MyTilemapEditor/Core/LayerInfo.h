#pragma once

#include <QString>

class LayerInfo
{
public:
	LayerInfo( const QString& name = "Layer 1", bool isLock = false, bool isVisible = true );
	~LayerInfo();

	QString		getNmae() const { return m_name; }
	bool		isLock() const { return m_isLock; }
	bool		isVisible() const { return m_isVisible; }

	void		setName( const QString& name ) { m_name = name; }
	void		setIsLock( bool isLock ) { m_isLock = isLock; }
	void		setIsVisible( bool isVisible ) { m_isVisible = isVisible; }
private:
	QString m_name;
	bool m_isLock = false;
	bool m_isVisible = true;
};