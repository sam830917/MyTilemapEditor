#include "Core//IntInput.h"

IntInput::IntInput()
	:QSpinBox()
{
	setMinimum( -2147483648 );
	setMaximum( 2147483647 );

}

IntInput::IntInput( int* connectInt )
	: QSpinBox()
{
	setMinimum( -2147483648 );
	setMaximum( 2147483647 );
	setValue( *connectInt );
}

IntInput::~IntInput()
{

}
