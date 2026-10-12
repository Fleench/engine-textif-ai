#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <string>
#include <map>
#include <functional>
#include <cctype>

struct Action;
class Game;

class GameObject {
public:
    using VerbFn = std::function<void(Game&, GameObject*, const Action&)>;
    std::map<std::string, VerbFn> verbs;

    void supports(Game& game, const std::string& name);
    void supports(Game& game, const std::string& name, VerbFn func);

    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, bool> flags;

    GameObject(const std::string& n) : id(n), name(n) {}
    GameObject(const std::string& i, const std::string& n) : id(i), name(n) {}
    virtual ~GameObject() = default;

    GameObject& setDescription(const std::string& desc) {
        description = desc;
        return *this;
    }

    std::string getDisplayName() const { return name.empty() ? id : name; }
    std::string getMatchName() const {
        std::string s = getDisplayName();
        for (auto& c : s) c = std::tolower((unsigned char)c);
        return s;
    }

    GameObject& setFlag(const std::string& key, bool val = true) {
        flags[key] = val;
        return *this;
    }
};

#endif
