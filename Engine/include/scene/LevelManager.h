#include <vector>
#include <stack>
#include <memory>
#include "./scene/GameObject.h"

class EntityPool {
public:
    explicit EntityPool(size_t capacity);

    //puts a gameobject into the entitypool
    //Args&&... args is basically the arguments for any entity constructor
    //T MUST be a type deriveved from GameObject
    template<typename T, typename... Args>
    T* Spawn(Args&&... args) {
        static_assert(std::is_base_of<GameObject, T>::value, "T must be derived from GameObject");

        if (freeList.empty()) //no more entities can be spawned
        {
            return nullptr;
        }

        size_t index = freeList.top();
        freeList.pop();

        pool[index] = std::make_unique<T>(std::forward<Args>(args)...);
        pool[index].get()->set_ID(index);

        return static_cast<T*>(pool[index].get());
    }

    void Kill(size_t index);

    void Render();

    void UpdateAll(float dt);

private:
    std::vector<std::unique_ptr<GameObject>> pool;
    std::stack<size_t> freeList;
};