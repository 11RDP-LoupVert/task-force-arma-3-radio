#pragma once
#include "clientData.hpp"
#include <Windows.h>
#include "Locks.hpp"



class serverData {
    friend class serverDataDirectory;
public:



    bool hasClientData(TSClientID clientID) const;
    std::shared_ptr<clientData> getClientData(const std::string& nickname) const;
    std::shared_ptr<clientData> getClientData(TSClientID clientID) const;
	void forEachClient(std::function<void(const std::shared_ptr<clientData>&)> func);
    std::shared_ptr<clientData> myClientData;//No lock needed. Its only changed once on serverconnect

    serverData() {

    }
private:
    using LockGuard_shared = LockGuard_shared<ReadWriteLock>;
    using LockGuard_exclusive = LockGuard_exclusive<ReadWriteLock>;
    mutable ReadWriteLock m_lock;
    void clientJoined(TSClientID clientID, const std::string& clientNickname);
    void clientLeft(TSClientID clientID);
    void clientUpdated(TSClientID clientID, const std::string& clientNickname);

    //Should really make a template out of this..... Buuut... meh
    using indexedType = std::string;//Is always the first element in tuple and always hashed
    using nonIndexedType = TSClientID;//Is always the second element in tuple
    using clientDataDirectoryElement = std::tuple<size_t, TSClientID, std::shared_ptr<clientData>>;

    std::vector<clientDataDirectoryElement> data;
    //Don't worry about these... This stuff is almost 50% faster than using std::map and std::unordered_map

    void debugPrint(std::stringstream& diag) const;

    auto findClientData(const indexedType& idx) const {
        auto range = std::equal_range(data.begin(), data.end(), std::make_tuple(std::hash<indexedType>()(idx), 0, nullptr), [](const auto& lhs, const auto& rhs) {
            return std::get<0>(lhs) < std::get<0>(rhs);
        });
        if (range.first == range.second) return data.end();
        return range.first;
    }

    auto findClientData(const nonIndexedType& cid) const {
        return std::find_if(data.begin(), data.end(), [&cid](const auto& lhs) {
            return std::get<1>(lhs) == cid;
        });
    }

    auto findClientData(const nonIndexedType& cid) {//We need a non-const to modify the value in iterator. But still need the const one for calls that don't modify
        return std::find_if(data.begin(), data.end(), [&cid](const auto& lhs) {
            return std::get<1>(lhs) == cid;
        });
    }

    auto containsClientData(const nonIndexedType& cid) const {
        for (auto& it : data) {
            if (std::get<1>(it) == cid)
                return true;
       }
        return false;
    }

    template <typename ...T>
    auto insertInData(const indexedType& idx, T &&... args) {
        auto hash = std::hash<indexedType>()(idx);
        return data.emplace(std::upper_bound(data.begin(), data.end(), hash, [](const auto& hash, const auto& tup) {
            return hash < std::get<0>(tup);
        }), hash, std::forward<T>(args)...);
    };


};



class serverDataDirectory {
public:
    serverDataDirectory();

    std::shared_ptr<serverData> getClientDataDirectory(TSServerID serverID) const;
    bool hasDirectory(TSServerID serverConnectionHandlerID) const;
private:
    using LockGuard_shared = LockGuard_shared<ReadWriteLock>;
    using LockGuard_exclusive = LockGuard_exclusive<ReadWriteLock>;
    mutable ReadWriteLock m_lock;
    std::map<TSServerID, std::shared_ptr<serverData>> data;

};












