#pragma once

#include <QSpinBox>

class IntInput : public QSpinBox
{
public:
	IntInput();
	IntInput( int* connectInt );
	~IntInput();

private:
	int* m_connectInt;
};