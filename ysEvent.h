#pragma once

#include <unordered_set>

template<class _EventOwner, typename... _Params>
class Event
{
public:
    using EventType = void (*)(_Params...);

    Event() = default;
    Event(Event const&) = delete;
    Event(Event&&) = delete;
    ~Event() = default;
    Event& operator=(Event const&) = delete;
    Event& operator=(Event&&) = delete;

    void operator()(_Params... params) { for (auto iter : m_listeners) { iter(params...); } }
    Event& operator+=(EventType pEvent) { if (pEvent != nullptr) m_listeners.insert(pEvent); return *this; }
    void AddListener(EventType pEvent) { this += pEvent; }

private:
    Event& operator=(EventType pEvent) { m_listeners.clear(); this += pEvent; return *this; }
    Event& operator-=(EventType pEvent)
    {
        if (auto i = m_listeners.find(pEvent); i != m_listeners.end())
            m_listeners.erase(i);
        return *this;
    }
    void RemoveListener(EventType pEvent) { this -= pEvent; }
    void RemoveAllListener() { m_listeners.clear(); }
    void Invoke(_Params... params) { for (auto iter : m_listeners) { iter(params...); } }

    std::unordered_multiset<EventType> m_listeners;

    friend _EventOwner;
};
template<class _EventOwner>
class Event<_EventOwner, void>
{
public:
    using EventType = void (*)(void);

    Event() = default;
    Event(Event const&) = delete;
    Event(Event&&) = delete;
    ~Event() = default;
    Event& operator=(Event const&) = delete;
    Event& operator=(Event&&) = delete;

    void operator()() { for (auto iter : m_listeners) { iter(); } }
    Event& operator+=(EventType pEvent) { if (pEvent != nullptr) m_listeners.insert(pEvent); return *this; }
    void AddListener(EventType pEvent) { this += pEvent; }

private:
    Event& operator=(EventType pEvent) { m_listeners.clear(); this += pEvent; return *this; }
    Event& operator-=(EventType pEvent)
    {
        if (auto i = m_listeners.find(pEvent); i != m_listeners.end())
            m_listeners.erase(i);
        return *this;
    }
    void RemoveListener(EventType pEvent) { this -= pEvent; }
    void RemoveAllListener() { m_listeners.clear(); }
    void Invoke() { (*this)(); }

    std::unordered_multiset<EventType> m_listeners;

    friend _EventOwner;
};