#pragma once

#include <boost/noncopyable.hpp>

namespace curl {

class Init final: private boost::noncopyable {
	
public:
	Init();
	~Init();
};

}
