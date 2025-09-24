#include "./scene/LevelManager.h"

EntityPool::EntityPool(size_t capacity) :pool(capacity) {
    for (size_t i = 0; i < capacity; ++i) {
        freeList.push(i);
    }
}

void  EntityPool::Kill(size_t idx){

}

void  EntityPool::Render(){
}

void  EntityPool::UpdateAll(float dt){
}
