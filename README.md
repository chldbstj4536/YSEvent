# YSEvent
**이벤트 라이브러리**

다양한 종류의 함수형식에 대한 이벤트를 간편하게 생성할 수 있다.
```c++
Event<bool(int, int)> event;                // 반환형이 bool, 매개변수로 int를 두개 받는 이벤트 생성
event += SomeFunc;                          // bool SomeFunc(int, int) 함수 등록
event.AddListener(foo, &foo::SomeFunc);     // foo의 맴버함수 등록
event();                                    // 등록된 이벤트 호출
```

## 요구 사항
- C++20 이상 컴파일러
- [CMake][cmakelink] (3.25버전 이상)

[cmakelink]: https://cmake.org/install/

## 사용 방법

CMake에서 빌드 후 라이브러리 사용

## CMake에서 빌드 방법

YSEvent 저장소를 복제한다.

```
git clone --recurse-submodules https://github.com/chldbstj4536/YSEvent.git
```

저장소로 이동한다.

```
cd YSEvent
```

cmake 명령어를 통해 build 폴더에 프로젝트를 생성한다.

```
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="./out"
```

cmake 명령어를 통해 생성된 프로젝트를 빌드하고 결과물을 out폴더에 설치한다.
```
cmake --build build --target install
```

lib/inc 폴더에 헤더와 라이브러리가 생성된다.

## 테스트 프로젝트 생성 방법

라이브러리 설치 방법을 따라서 설치를 먼저 한다.

test 폴더로 이동한다.

```
cd test
```

test 프로젝트를 생성한다.

```
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="./out"
```

위까지 진행하면 자신의 플렛폼에 설치된 IDE에 맞게 build 폴더에 프로젝트 파일이 생성된다.

cmake 명령어를 통해 생성된 테스트 프로젝트를 빌드한다.

``` cmake
cmake --build build
```

build/Debug 폴더 안에 테스트 프로그램 실행파일이 생성되어있다.

아래 명령어를 통해 예제 프로그램 결과를 확인할 수 있다.

```
build/Debug/YSEventTest.exe
```