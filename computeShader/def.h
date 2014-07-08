#ifndef INCLUDE_DEF_H_
#define INCLUDE_DEF_H_

#define SAFE_RELEASE( x ) if( x != nullptr ){ x->Release(); x = nullptr; }

#endif