#include <list>
#include <memory>
#include <exception>
#include <concepts>

namespace YS
{
    template <typename T>
    concept non_void = !std::is_void_v<T>;

    template <typename _FuncType>
    class Event;

    template <typename _ReturnType, typename... _ParamTypes>
    class Event<_ReturnType(_ParamTypes...)>
    {
        using EventFnType = _ReturnType (*)(_ParamTypes...);
        template <class _Member>
        using EventMemFnType = _ReturnType (_Member::*)(_ParamTypes...);
        template <class _Member>
        using EventConstMemFnType = _ReturnType (_Member::*)(_ParamTypes...) const;

#pragma region Define Function
        class Function
        {
        public:
            virtual _ReturnType operator()(_ParamTypes... params) = 0;
            virtual bool operator==(Function const &fn) = 0;
        };
        class NonMemFunction : public Function
        {
        public:
            NonMemFunction(EventFnType fn) : fn(fn) {}
            virtual _ReturnType operator()(_ParamTypes... params) override { return fn(params...); }
            virtual bool operator==(Function const &rhs) override
            {
                try { return dynamic_cast<NonMemFunction const &>(rhs).fn == fn; }
                catch (std::bad_cast e) { return false; }
            }

        private:
            EventFnType fn;
        };
        template <class _FnOwner>
        class MemFunction : public Function
        {
        public:
            MemFunction(std::shared_ptr<_FnOwner> const &pOwner, EventMemFnType<_FnOwner> fn) : pOwner(pOwner), fn(fn) {}
            virtual _ReturnType operator()(_ParamTypes... params) override
            {
                if (auto pShared = pOwner.lock())
                    return ((*pShared).*fn)(params...);
                throw std::bad_weak_ptr();
            }
            virtual bool operator==(Function const &rhs) override
            {
                try
                {
                    auto const &cast_fn = dynamic_cast<MemFunction const &>(rhs);
                    auto pOwnerLhs = pOwner.lock();
                    auto pOwnerRhs = cast_fn.pOwner.lock();
                    if (pOwnerLhs && pOwnerRhs)
                        return cast_fn.fn == fn && (*pOwnerLhs) == (*pOwnerRhs);
                    throw std::bad_weak_ptr();
                }
                catch (std::bad_cast e) { return false; }
            }

        private:
            std::weak_ptr<_FnOwner> pOwner;
            EventMemFnType<_FnOwner> fn;
        };
        template <class _FnOwner> class ConstMemFunction;
        template <class _FnOwner>
        class ConstMemFunction<const _FnOwner> : public Function
        {
        public:
            ConstMemFunction(std::shared_ptr<_FnOwner> const &pOwner, EventConstMemFnType<_FnOwner> fn) : pOwner(pOwner), fn(fn) {}
            virtual _ReturnType operator()(_ParamTypes... params) override
            {
                if (auto pShared = pOwner.lock())
                    return ((*pShared).*fn)(params...);
                throw std::bad_weak_ptr();
            }
            virtual bool operator==(Function const &rhs) const
            {
                try
                {
                    auto const &cast_fn = dynamic_cast<ConstMemFunction const &>(rhs);
                    auto pOwnerLhs = pOwner.lock();
                    auto pOwnerRhs = cast_fn.pOwner.lock();
                    if (pOwnerLhs && pOwnerRhs)
                        return cast_fn.fn == fn && (*pOwnerLhs) == (*pOwnerRhs);
                    throw std::bad_weak_ptr();
                }
                catch (std::bad_cast e) { return false; }
            }

        private:
            std::weak_ptr<_FnOwner> pOwner;
            EventConstMemFnType<_FnOwner> fn;
        };
#pragma endregion

    public:
        Event() = default;
        Event(Event const &) = delete;
        Event(Event &&) = delete;
        ~Event() = default;
        Event &operator=(Event const &) = delete;
        Event &operator=(Event &&) = delete;

        std::list<_ReturnType> operator()(_ParamTypes... params)
            requires(non_void<_ReturnType>)
        {
            std::list<_ReturnType> rvs;
            auto i = m_listeners.begin();
            while (i != m_listeners.end())
            {
                try { rvs.push_back((*(*i++))(params...)); }
                catch (std::bad_weak_ptr e) { i = m_listeners.erase(--i); }
            }
            return rvs;
        }
        void operator()(_ParamTypes... params)
        {
            auto i = m_listeners.begin();
            while (i != m_listeners.end())
            {
                try { (*(*i++))(params...); }
                catch (std::bad_weak_ptr e) { i = m_listeners.erase(--i); }
            }
        }
        Event &operator+=(EventFnType pFn)
        {
            if (pFn != nullptr)
                m_listeners.push_back(std::make_unique<NonMemFunction>(pFn));
            return *this;
        }
        Event &operator=(EventFnType pFn)
        {
            m_listeners.clear();
            this += pFn;
            return *this;
        }
        Event &operator-=(EventFnType pFn)
        {
            RemoveListener(NonMemFunction(pFn));
            return *this;
        }

        void AddListener(EventFnType pFn) { *this += pFn; }
        template <class _Owner>
        void AddListener(std::shared_ptr<_Owner> const &pOwner, EventMemFnType<_Owner> pMemFn)
        {
            if (pMemFn != nullptr)
                m_listeners.push_back(std::make_unique<MemFunction<_Owner>>(pOwner, pMemFn));
        }
        template <class _Owner>
        void AddListener(std::shared_ptr<const _Owner> const &pOwner, EventConstMemFnType<const _Owner> pMemFn)
        {
            if (pMemFn != nullptr)
                m_listeners.push_back(std::make_unique<ConstMemFunction<const _Owner>>(pOwner, pMemFn));
        }
        void RemoveListener(EventFnType pFn) { *this -= pFn; }
        template <class _Owner>
        void RemoveListener(std::shared_ptr<_Owner> const &_pOwner, EventMemFnType<_Owner> pMemFn)
        {
            RemoveListener(MemFunction(_pOwner, pMemFn));
        }
        template <class _Owner>
        void RemoveListener(std::shared_ptr<const _Owner> const &pOwner, EventConstMemFnType<_Owner> pConstMemFn)
        {
            RemoveListener(ConstMemFunction(pOwner, pConstMemFn));
        }
        void RemoveAllListener() { m_listeners.clear(); }

    private:
        void RemoveListener(Function const &fn)
        {
            auto iter = m_listeners.begin();
            while (iter != m_listeners.end())
            {
                if ((*iter)->operator==(fn))
                {
                    m_listeners.erase(iter);
                    break;
                }
                ++iter;
            }
        }
        std::list<std::unique_ptr<Function>> m_listeners;
    };
}