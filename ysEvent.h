#include <list>
#include <memory>
#include <exception>
#include <concepts>

namespace YS
{
    template <typename T>
    concept non_void = !std::is_void_v<T>;
    template <typename T>
    concept constant = std::is_const_v<T>;
    template <typename T>
    concept non_constant = !std::is_const_v<T>;

    template <class _R, typename... _Args> using FnPtr = _R(*)(_Args...);
    template <class _C, typename _R, typename... _Args> using MemFnPtr = _R(_C::*)(_Args...);
    template <class _C, typename _R, typename... _Args> using ConstMemFnPtr = _R(_C::*)(_Args...) const;
    template <class _C, typename _R, typename... _Args> MemFnPtr<_C, _R, _Args...> SelectNonConstFn(MemFnPtr<_C, _R, _Args...> fp) { return fp; }
    template <class _C, typename _R, typename... _Args> ConstMemFnPtr<_C, _R, _Args...> SelectConstFn(ConstMemFnPtr<_C, _R, _Args...> fp) { return fp; }

    template <typename _FuncType>
    class Event;

    template <typename _R, typename... _Args>
    class Event<_R(_Args...)>
    {
#pragma region Type Define
        using EventFnPtr = FnPtr<_R, _Args...>;
        template <class _C> using EventMemFnPtr = MemFnPtr<_C, _R, _Args...>;
        template <class _C> using EventConstMemFnPtr = ConstMemFnPtr<_C, _R, _Args...>;
#pragma endregion
#pragma region Define Function
        /// <summary>
        /// 이벤트에 등록될 함수를 담기 위한 기본 클래스
        /// </summary>
        class Function
        {
        public:
            virtual _R operator()(_Args... params) = 0;
            virtual bool operator==(Function const &rhs) const = 0;
        };
        /// <summary>
        /// 비멤버 함수를 담기 위한 파생 클래스
        /// </summary>
        class NonMemFunction : public Function
        {
        public:
            NonMemFunction(EventFnPtr pFn) : m_pFn(pFn) {}
            virtual _R operator()(_Args... args) override { return m_pFn(args...); }
            virtual bool operator==(Function const &rhs) const override
            {
                try { return dynamic_cast<NonMemFunction const &>(rhs).m_pFn == m_pFn; }
                catch (std::bad_cast e) { return false; }
            }
        private:
            EventFnPtr m_pFn;
        };
        /// <summary>
        /// 비상수 멤버 함수를 담기 위한 파생 클래스
        /// </summary>
        /// <typeparam name="_C">해당 함수를 보유하고 있는 클래스</typeparam>
        template <class _C>
        class MemFunction : public Function
        {
        public:
            MemFunction(std::shared_ptr<_C> const &pOwner, EventMemFnPtr<_C> pMemFn) : m_pOwner(pOwner), m_pMemFn(pMemFn) {}
            virtual _R operator()(_Args... args) override
            {
                if (auto pShared = m_pOwner.lock())
                    return ((*pShared).*m_pMemFn)(args...);
                throw std::bad_weak_ptr();
            }
            virtual bool operator==(Function const &rhs) const override
            {
                try
                {
                    auto const &rhsFn = dynamic_cast<MemFunction const &>(rhs);
                    auto pOwnerLhs = m_pOwner.lock();
                    auto pOwnerRhs = rhsFn.m_pOwner.lock();
                    if (pOwnerLhs && pOwnerRhs)
                        return rhsFn.m_pMemFn == m_pMemFn && pOwnerLhs == pOwnerRhs;
                    throw std::bad_weak_ptr();
                }
                catch (std::bad_cast e) { return false; }
            }

        private:
            std::weak_ptr<_C> m_pOwner;
            EventMemFnPtr<_C> m_pMemFn;
        };
        /// <summary>
        /// 상수 멤버 함수를 담기 위한 파생 클래스
        /// </summary>
        /// <typeparam name="_C">해당 함수를 보유하고 있는 클래스</typeparam>
        template <constant _C>
        class ConstMemFunction : public Function
        {
        public:
            ConstMemFunction(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<_C> pConstMemFn) : m_pOwner(pOwner), m_pConstMemFn(pConstMemFn) {}
            virtual _R operator()(_Args... args) override
            {
                if (auto pShared = m_pOwner.lock())
                    return ((*pShared).*m_pConstMemFn)(args...);
                throw std::bad_weak_ptr();
            }
            virtual bool operator==(Function const &rhs) const override
            {
                try
                {
                    auto const &rhsFn = dynamic_cast<ConstMemFunction const &>(rhs);
                    auto pOwnerLhs = m_pOwner.lock();
                    auto pOwnerRhs = rhsFn.m_pOwner.lock();
                    if (pOwnerLhs && pOwnerRhs)
                        return rhsFn.m_pConstMemFn == m_pConstMemFn && pOwnerLhs == pOwnerRhs;
                    throw std::bad_weak_ptr();
                }
                catch (std::bad_cast e) { return false; }
            }

        private:
            std::weak_ptr<_C> m_pOwner;
            EventConstMemFnPtr<_C> m_pConstMemFn;
        };
#pragma endregion
    public:
        Event() = default;
        Event(Event const &) = delete;
        Event(Event &&) = delete;
        ~Event() = default;
        Event& operator=(Event const &) = delete;
        Event& operator=(Event &&) = delete;

        /// <summary>
        /// 반환값이 존재하는 타입에 대한 함수 호출
        /// </summary>
        /// <param name="args">함수 호출에 필요한 매개변수</param>
        /// <returns>함수의 반환 값</returns>
        std::list<_R> operator()(_Args... args)
            requires(non_void<_R>)
        {
            std::list<_R> rvs;
            auto i = m_listeners.begin();
            while (i != m_listeners.end())
            {
                try { rvs.push_back((*(*i++))(args...)); }
                catch (std::bad_weak_ptr e) { i = m_listeners.erase(--i); }
            }
            return rvs;
        }
        /// <summary>
        /// 반환값이 존재하지 않는 함수 타입에 대한 함수 호출
        /// </summary>
        /// <param name="args">함수 호출에 필요한 매개변수</param>
        void operator()(_Args... args)
        {
            auto i = m_listeners.begin();
            while (i != m_listeners.end())
            {
                try { (*(*i++))(args...); }
                catch (std::bad_weak_ptr e) { i = m_listeners.erase(--i); }
            }
        }
        /// <summary>
        /// 이벤트에 함수 등록 (AddListener와 동일)
        /// 비멤버 함수에만 사용 가능
        /// </summary>
        /// <param name="pFn">등록할 함수 포인터</param>
        /// <returns>자기 자신을 반환</returns>
        Event& operator+=(EventFnPtr pFn)
        {
            if (pFn != nullptr)
                m_listeners.push_back(std::make_unique<NonMemFunction>(pFn));
            return *this;
        }
        /// <summary>
        /// 이벤트에 등록된 모든 함수를 제거하고 pFn 함수 등록
        /// </summary>
        /// <param name="_R">등록할 함수 포인터</typeparam>
        /// <returns>자기 자신을 반환</returns>
        Event& operator=(EventFnPtr pFn)
        {
            m_listeners.clear();
            this += pFn;
            return *this;
        }
        /// <summary>
        /// pFn을 이벤트에서 등록 해제
        /// </summary>
        /// <param name="_R">등록 해제할 함수 포인터</typeparam>
        /// <returns>자기 자신을 반환</returns>
        Event& operator-=(EventFnPtr pFn)
        {
            RemoveListener(NonMemFunction(pFn));
            return *this;
        }

        /// <summary>
        /// 일반 함수 등록
        /// </summary>
        /// <param name="pFn">등록할 함수 포인터</param>
        void AddListener(EventFnPtr pFn) { *this += pFn; }
        /// <summary>
        /// 비상수 객체로부터 비상수 멤버 함수 등록
        /// </summary>
        /// <param name="pOwner">비상수 멤버 함수를 호출할 비상수 객체</param>
        /// <param name="pMemFn">등록할 비상수 멤버 함수</param>
        template <non_constant _C>
        void AddListener(std::shared_ptr<_C> const &pOwner, EventMemFnPtr<_C> pMemFn)
        {
            if (pMemFn != nullptr)
                m_listeners.push_back(std::make_unique<MemFunction<_C>>(pOwner, pMemFn));
        }
        /// <summary>
        /// 비상수 객체로부터 상수 멤버 함수 등록
        /// </summary>
        /// <param name="pOwner">상수 멤버 함수를 호출할 비상수 객체</param>
        /// <param name="pConstMemFn">등록할 상수 멤버 함수</param>
        template <non_constant _C>
        void AddListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<_C> pConstMemFn)
        {
            if (pConstMemFn != nullptr)
                m_listeners.push_back(std::make_unique<ConstMemFunction<const _C>>(pOwner, pConstMemFn));
        }
        /// <summary>
        /// 상수 객체로부터 상수 멤버 함수 등록
        /// </summary>
        /// <param name="pOwner">상수 멤버 함수를 호출할 상수 객체</param>
        /// <param name="pConstMemFn">등록할 상수 멤버 함수</param>
        template <constant _C>
        void AddListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<std::remove_const_t<_C>> pConstMemFn)
        {
            if (pConstMemFn != nullptr)
                m_listeners.push_back(std::make_unique<ConstMemFunction<_C>>(pOwner, pConstMemFn));
        }
        /// <summary>
        /// 일반 함수 등록 해제
        /// </summary>
        /// <param name="pFn">등록된 함수</param>
        void RemoveListener(EventFnPtr pFn) { *this -= pFn; }
        /// <summary>
        /// 비상수 객체로 등록된 비상수 멤버 함수 등록 해제
        /// </summary>
        /// <typeparam name="_Owner">등록한 비상수 객체 타입</typeparam>
        /// <param name="pOwner">등록한 객체</param>
        /// <param name="pConstMemFn">등록된 비상수 멤버 함수</param>
        template <non_constant _C>
        void RemoveListener(std::shared_ptr<_C> const &_pOwner, EventMemFnPtr<_C> pMemFn)
        {
            RemoveListener(MemFunction(_pOwner, pMemFn));
        }
        /// <summary>
        /// 비상수 객체로 등록된 상수 멤버 함수 등록 해제
        /// </summary>
        /// <typeparam name="_Owner">등록한 비상수 객체 타입</typeparam>
        /// <param name="pOwner">등록한 객체</param>
        /// <param name="pConstMemFn">등록된 상수 멤버 함수</param>
        template <non_constant _C>
        void RemoveListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<_C> pConstMemFn)
        {
            RemoveListener(ConstMemFunction<const _C>(pOwner, pConstMemFn));
        }
        /// <summary>
        /// 상수 객체로 등록된 상수 멤버 함수 등록 해제
        /// </summary>
        /// <typeparam name="_Owner">등록한 상수 객체 타입</typeparam>
        /// <param name="pOwner">등록한 객체</param>
        /// <param name="pConstMemFn">등록된 상수 멤버 함수</param>
        template <constant _C>
        void RemoveListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<std::remove_const_t<_C>> pConstMemFn)
        {
            RemoveListener(ConstMemFunction<const _C>(pOwner, pConstMemFn));
        }
        /// <summary>
        /// 등록된 모든 함수들 삭제
        /// </summary>
        void RemoveAllListener() { m_listeners.clear(); }

    private:
        void RemoveListener(Function const &fn)
        {
            auto iter = m_listeners.begin();
            while (iter != m_listeners.end())
            {
                if ((*(*iter)) == (fn))
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