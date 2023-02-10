/**
 * @file ysEvent.h
 * @author 최윤서 (choicoco1995@naver.com)
 * @brief 이벤트 시스템
 * @version 1.0.0
 * @date 2023-01-30
 */
#include <list>
#include <memory>
#include <exception>
#include <concepts>

namespace YS
{
    /**
     * @brief T를 void타입으로 제약하는 콘셉트
     */
    template <typename T>
    concept non_void = !std::is_void_v<T>;
    /**
     * @brief T를 상수형으로 제약하는 콘셉트
     */
    template <typename T>
    concept constant = std::is_const_v<T>;
    /**
     * @brief T를 비상수형으로 제약하는 콘셉트
     */
    template <typename T>
    concept non_constant = !std::is_const_v<T>;

    /**
     * @brief 함수 포인터 타입 재정의
     * 
     * @tparam _R 반환 타입
     * @tparam _Args 매개변수 타입
     */
    template <class _R, typename... _Args> using FnPtr = _R(*)(_Args...);
    /**
     * @brief 비 상수 멤버 함수 포인터 타입 재정의
     * 
     * @tparam _C 함수 호출 객체 타입
     * @tparam _R 반환 타입
     * @tparam _Args 매개변수 타입
     */
    template <class _C, typename _R, typename... _Args> using MemFnPtr = _R(_C::*)(_Args...);
    /**
     * @brief 상수 멤버 함수 포인터 타입 재정의
     * 
     * @tparam _C 함수 호출 객체 타입
     * @tparam _R 반환 타입
     * @tparam _Args 매개변수 타입
     */
    template <class _C, typename _R, typename... _Args> using ConstMemFnPtr = _R(_C::*)(_Args...) const;
    /**
     * @brief 비상수 맴버 함수 포인터를 선택하는 함수
     * 
     * 이벤트 시스템 사용 중 상수 멤버 함수와 비상수 멤버 함수의 이름이 같은 경우 비상수 객체에 대해 Event::AddListener 사용 시\n
     * 컴파일러는 상수 멤버 함수와 비상수 멤버 함수 중 어떤 걸 선택해야 하는지 컴파일러가 정할 수 없다.\n
     * 따라서 명시적 캐스팅을 해줘야 하는데, 이를 간편하게 해주기 위한 헬퍼 함수이다.
     * 
     * @param fp 선택할 멤버 함수 포인터
     * @return MemFnPtr<_C, _R, _Args...> 비상수 멤버 함수 포인터를 반환한다.
     */
    template <class _C, typename _R, typename... _Args> MemFnPtr<_C, _R, _Args...> SelectNonConstFn(MemFnPtr<_C, _R, _Args...> fp) { return fp; }
    /**
     * @brief 상수 맴버 함수 포인터를 선택하는 함수
     * 
     * SelectNonConstFn 함수와 같다.
     * 
     * @param fp 선택할 멤버 함수 포인터
     * @return MemFnPtr<_C, _R, _Args...> 상수 멤버 함수 포인터를 반환한다.
     */
    template <class _C, typename _R, typename... _Args> ConstMemFnPtr<_C, _R, _Args...> SelectConstFn(ConstMemFnPtr<_C, _R, _Args...> fp) { return fp; }

    /**
     * @brief 기반 이벤트 클래스
     * 
     * 함수의 반환 타입과 매개변수 타입을 분리해서 구현해야 하지만 사용자가 사용할 때는\n
     * 함수타입 하나로도 간편하게 사용할 수 있도록 하기 위한 전방선언 클래스이다.
     * 
     * @tparam _FuncType 함수의 타입
     */
    template <typename _FuncType>
    class Event;

    /**
     * @brief 이벤트 클래스
     * 
     * 반환 타입과 매개변수 타입을 가진 이벤트 클래스
     * 
     * @tparam _R 반환 타입
     * @tparam _Args 매개변수 타입
     */
    template <typename _R, typename... _Args>
    class Event<_R(_Args...)>
    {
#pragma region Type Define
        using EventFnPtr = FnPtr<_R, _Args...>;
        template <class _C> using EventMemFnPtr = MemFnPtr<_C, _R, _Args...>;
        template <class _C> using EventConstMemFnPtr = ConstMemFnPtr<_C, _R, _Args...>;
#pragma endregion
#pragma region Define Function
        /**
         * @brief 이벤트에 등록될 함수를 담기 위한 기본 클래스
         */
        class Function
        {
        public:
            virtual _R operator()(_Args... params) = 0;
            virtual bool operator==(Function const &rhs) const = 0;
            virtual std::unique_ptr<Function> Clone() const = 0;
        };
        /**
         * @brief 비멤버 함수를 담기 위한 파생 클래스
         */
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
            virtual std::unique_ptr<Function> Clone() const override { return std::make_unique<NonMemFunction>(m_pFn); }
        private:
            EventFnPtr m_pFn;
        };
        /**
         * @brief 비상수 멤버 함수를 담기 위한 파생 클래스
         * 
         * @tparam _C 해당 함수를 보유하고 있는 클래스
         */
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
            virtual std::unique_ptr<Function> Clone() const override
            { return std::make_unique<MemFunction>(m_pOwner.lock(), m_pMemFn); }

        private:
            std::weak_ptr<_C> m_pOwner;
            EventMemFnPtr<_C> m_pMemFn;
        };

        /**
         * @brief 상수 멤버 함수를 담기 위한 파생 클래스
         * 
         * @tparam _C 해당 함수를 보유하고 있는 클래스
         */
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
            virtual std::unique_ptr<Function> Clone() const override
            { return std::make_unique<ConstMemFunction>(m_pOwner.lock(), m_pConstMemFn); }

        private:
            std::weak_ptr<_C> m_pOwner;
            EventConstMemFnPtr<_C> m_pConstMemFn;
        };
#pragma endregion
    public:
/// @cond
        Event() = default;
        Event(Event const &o) { *this = o; }
        Event(Event &&) = default;
        ~Event() = default;
        Event& operator=(Event const &o)
        {
            for (auto& listener : o.m_listeners)
                m_listeners.push_back(listener->Clone());
            return *this;
        }
        Event& operator=(Event &&) = default;
/// @endcond

        /**
         * @brief 반환값이 존재하는 타입에 대한 함수 호출
         * 
         * @param args 함수 호출에 필요한 매개변수
         * @return 함수의 반환 값
        */
        std::list<_R> operator()(_Args... args) const
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
        /**
         * @brief 반환값이 존재하지 않는 함수 타입에 대한 함수 호출
         * 
         * @param args 함수 호출에 필요한 매개변수
         */
        void operator()(_Args... args) const
        {
            auto i = m_listeners.begin();
            while (i != m_listeners.end())
            {
                try { (*(*i++))(args...); }
                catch (std::bad_weak_ptr e) { i = m_listeners.erase(--i); }
            }
        }
        /**
         * @brief 이벤트에 함수 등록
         * 
         * AddListner와 동일\n
         * 비멤버 함수에만 사용가능
         * 
         * @param pFn 등록할 함수 포인터
         * @return Event& 자기 자신을 반환
         */
        Event& operator+=(EventFnPtr pFn)
        {
            if (pFn != nullptr)
                m_listeners.push_back(std::make_unique<NonMemFunction>(pFn));
            return *this;
        }
        /**
         * @brief 이벤트에 등록된 모든 함수를 제거하고 pFn 함수 등록 
         * 
         * @param pFn 등록할 함수 포인
         * @return Event& 자기 자신을 반환
         */
        Event& operator=(EventFnPtr pFn)
        {
            m_listeners.clear();
            this += pFn;
            return *this;
        }
        /**
         * @brief pFn을 이벤트에서 등록 해제
         * 
         * @param pFn 등록 해제할 함수 포인터
         * @return Event& 자기 자신을 반환
         */
        Event& operator-=(EventFnPtr pFn)
        {
            RemoveListener(NonMemFunction(pFn));
            return *this;
        }

        /**
         * @brief 일반 함수 등록
         * 
         * @param pFn 등록할 함수 포인터
         */
        void AddListener(EventFnPtr pFn) { *this += pFn; }
        /**
         * @brief 비상수 객체로부터 비상수 멤버 함수 등록
         * 
         * @param pOwner 비상수 멤버 함수를 호출할 비상수 객체
         * @param pMemFn 등록할 비상수 멤버 함수
         */
        template <non_constant _C>
        void AddListener(std::shared_ptr<_C> const &pOwner, EventMemFnPtr<_C> pMemFn)
        {
            if (pMemFn != nullptr)
                m_listeners.push_back(std::make_unique<MemFunction<_C>>(pOwner, pMemFn));
        }
        /**
         * @brief 비상수 객체로부터 상수 멤버 함수 등록
         * 
         * @param pOwner 상수 멤버 함수를 호출할 비상수 객체
         * @param pConstMemFn 등록할 상수 멤버 함수
         */
        template <non_constant _C>
        void AddListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<_C> pConstMemFn)
        {
            if (pConstMemFn != nullptr)
                m_listeners.push_back(std::make_unique<ConstMemFunction<const _C>>(pOwner, pConstMemFn));
        }
        /**
         * @brief 상수 객체로부터 상수 멤버 함수 등록
         * 
         * @param pOwner 상수 멤버 함수를 호출할 상수 객체
         * @param pConstMemFn 등록할 상수 멤버 함수
         */
        template <constant _C>
        void AddListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<std::remove_const_t<_C>> pConstMemFn)
        {
            if (pConstMemFn != nullptr)
                m_listeners.push_back(std::make_unique<ConstMemFunction<_C>>(pOwner, pConstMemFn));
        }
        /**
         * @brief 일반 함수 등록 해제
         * 
         * @param pFn 등록된 함수
         */
        void RemoveListener(EventFnPtr pFn) { *this -= pFn; }
        /**
         * @brief 비상수 객체로 등록된 비상수 멤버 함수 등록 해제
         * 
         * @tparam _C 등록한 비상수 객체 타입
         * @param _pOwner 등록한 객체
         * @param pMemFn 등록된 비상수 멤버 함수
         */
        template <non_constant _C>
        void RemoveListener(std::shared_ptr<_C> const &_pOwner, EventMemFnPtr<_C> pMemFn)
        {
            RemoveListener(MemFunction(_pOwner, pMemFn));
        }
        /**
         * @brief 비상수 객체로 등록된 상수 멤버 함수 등록 해제
         *
         * @tparam _C 등록한 비상수 객체 타입
         * @param pOwner 등록한 객체
         * @param pConstMemFn 등록된 상수 멤버 함수
         * @return template <non_constant _C> 
         */
        template <non_constant _C>
        void RemoveListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<_C> pConstMemFn)
        {
            RemoveListener(ConstMemFunction<const _C>(pOwner, pConstMemFn));
        }
        /**
         * @brief 상수 객체로 등록된 상수 멤버 함수 등록 해제
         * 
         * @tparam _C 등록한 상수 객체 타입
         * @param pOwner 등록한 객체
         * @param pConstMemFn 등록된 상수 멤버 함수
         */
        template <constant _C>
        void RemoveListener(std::shared_ptr<_C> const &pOwner, EventConstMemFnPtr<std::remove_const_t<_C>> pConstMemFn)
        {
            RemoveListener(ConstMemFunction<const _C>(pOwner, pConstMemFn));
        }
        /**
         * @brief 등록된 모든 함수들 삭제
         */
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
        mutable std::list<std::unique_ptr<Function>> m_listeners;
    };
}