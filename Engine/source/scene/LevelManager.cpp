/* !------------------------------------------------------------------------------------------------
	File: LevelManager.cpp
	Project : NocturnalStudio

	This file contains ...

	Primary Author :
		Name(%)
		- Brief

	Secondary Author :
		Name(%)
		- Brief

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "./scene/LevelManager.h"

EntityPool::EntityPool(size_t capacity) :pool(capacity) {
    for (size_t i = 0; i < capacity; ++i) {
        freeList.push(i);
    }
}

void  EntityPool::Kill(size_t idx){

}

void EntityPool::Render(){
}

void  EntityPool::UpdateAll(float dt){
}
