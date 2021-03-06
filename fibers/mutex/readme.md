# Мьютекс

## Пререквизиты

- [fibers/coroutine](/tasks/fibers/coroutine)
- [fibers/sleep_for](/tasks/fibers/sleep_for)
- [mutex/mutex](/tasks/mutex/mutex)
- [condvar/condvar](/tasks/condvar/condvar)

---

Реализуйте примитивы синхронизации для файберов:
- Мьютекс ([`Mutex`](exe/fibers/sync/mutex.hpp))
- Условную переменную ([`CondVar`](exe/fibers/sync/condvar.hpp))
- [`WaitGroup`](exe/fibers/sync/wait_group.hpp)

Методы `Mutex::Lock`, `CondVar::Wait` и `WaitGroup::Wait` должны блокировать только вызвавший их файбер, но не блокировать поток планировщика, в котором этот файбер исполняется.

## Пример

```cpp
void SyncExample() {
  using namespace exe;
  
  tp::ThreadPool scheduler{/*threads=*/4};
  
  fibers::Go(scheduler, []() {
    fibers::Mutex mutex;
    size_t cs = 0;
    
    // https://gobyexample.com/waitgroups
    // Будем считать, что WaitGroup одноразовый, т.е. счетчик
    // опускается до нуля только один раз.
    fibers::WaitGroup wg;
    
    for (size_t i = 0; i < 123; ++i) {
      wg.Add(1);
      
      fibers::Go([&]() {
        // При выходе из скоупа будет вызван wg.Done()
        // https://gobyexample.com/defer
        wheels::Defer defer([&wg]() {
          wg.Done();
        });
        
        for (size_t j = 0; j < 1024; ++j) {
          std::lock_guard guard(mutex);
          ++cs;  // <-- в критической секции
        }
      });
    }
    
    // Дожидаемся завершения всех запущенных выше файберов
    wg.Wait();
    
    std::cout << cs << std::endl;
    // <-- Напечатано 123 * 1024
  });
  
  scheduler.WaitIdle();
  scheduler.Stop();
}
```

## `WaitGroup`

[Go by Example: WaitGroups](https://gobyexample.com/waitgroups)

`WaitGroup` – счетчик абстрактной "работы", например, запущенных файберов.

### Методы

Изначально в счетчике 0.

- `void Add(size_t count)` – добавить положительный `count` к значению счетчика
- `void Done()` – уменьшить значение счетчика на 1
- `void Wait()` – ждать (заблокировав файбер), пока значение счетчика не опустится до нуля

### Правила использования

Методы можно вызывать конкурентно. 

Вызовы `Add` должны быть упорядочены (через _happens-before_) с последним вызовом `Done` самим _пользователем_ `WaitGroup` .

Можно считать, что `WaitGroup` – "одноразовый", т.е. счетчик опускается до нуля только один раз. 

## `FutexLike`

Реализация примитивов синхронизации не будет отличаться для файберов и для потоков, вы можете перенести реализации из уже решенных задач.

Но в этот раз у вас не будет фьютекса. Для файберов его придется реализовать самостоятельно.

Реализуйте класс [`FutexLike<T>`](exe/fibers/sync/futex.hpp) – очередь ожидания для файберов, привязанная к произвольному атомику.

API:

* Конструктор `FutexLike` принимает ссылку на атомик.
* `ParkIfEqual(T old)` – атомарно (относительно конкурирующих вызовов `WakeOne` / `WakeAll`) сравнить текущее значение атомика с `old` и в случае равенства запарковать текущий файбер в очереди ожидания.
* `WakeOne` / `WakeAll` – разбудить один / все запаркованные в очереди файберы

### Реализация `FutexLike`

[kernel/futex/waitwake.c](https://github.com/torvalds/linux/blob/master/kernel/futex/waitwake.c)

#### Спинлок

Для синхронизации доступа к очереди файберов из конкурентных вызовов `ParkIfEqual` и `WakeOne` / `WakeAll` используйте [спинлок](exe/support/spinlock.hpp).

Напишите в спинлоке оптимизацию с чтениями в цикле ожидания (_Test-and-TAS_) для предотвращения cache ping-pong.

Расставьте оптимальные `memory_order`-ы.

#### Гонка

В `ParkIfEqual` вам потребуется разрешить гонку между `Suspend` и `Resume` файбера.

Для этого адаптируйте под фьютекс функцию `Suspend`.

## `Suspend`

Блокировка файбера на примитиве синхронизации на основе фьютекса – лишь частный случай общей задачи: файберу нужно остановиться и запланировать свое возобновление.
 
Примеры:
- `ParkIfEqual` на фьютексе: запланировать возобновление – значит положить себя в очередь ожидания, из которой нас потом достанут и запустят в вызове `WakeOne` / `WakeAll`.
- `Yield`: тут файбер моментально планирует себя в пул потоков.
- Блокирующий `Await` на [`Future`](/tasks/futures/futures): запланировать возобновление – подписаться на готовность `Future` с помощью `Subscribe` и запустить себя в коллбэке.
- [`Select`](/tasks/fibers/channel) на каналах

### Гонка

Во всех этих примерах есть потенциальная гонка между возобновлением файбера и его остановкой.

### Универсальность

В этой задаче мы хотим придумать универсальный `Suspend`, который подойдет для всех перечисленных выше применений.

Файбер не должен знать про конкретные примитивы синхронизации и блокирующие операции, которые с ним работают, про детали их реализации (например, про спинлок фьютекса).

### Awaiters

Решение – кастомизировать для `Suspend` планирование возобновления файбера с помощью внешней (по отношению к файберу) стратегии, которую определяет конкретный примитив синхронизации / блокирующая операция.

Эту стратегию мы назовем _Awaiter_.

_Awaiter_ решает когда именно остановленный файбер должен запуститься.

[Understanding operator co_await](https://lewissbaker.github.io/2017/11/17/understanding-operator-co-await)

### `FiberHandle`

Awaiter работает с _остановленным_ файбером не напрямую, а через объект [`FiberHandle`](exe/fibers/core/handle.hpp).

Непрозрачный `FiberHandle` позволит избавиться в реализации `FutexLike` от непосредственного доступа к объекту `Fiber`, скрыть его от пользователя.


## `Yield`

Выразите через `Suspend` функцию `Yield`. 

Так вы убедитесь, что получившийся `Suspend` подходит для разных задач.

## Аллокации

### `Suspend`

Метод `Yield`, реализованный через `Suspend`, должен обходиться без динамических аллокаций памяти.

Заворачивание лямбды в `std::function` или подобный контейнер с type erasure – тоже динамическая аллокация.

### `FutexLike`

В реализации фьютекса не должно быть динамических аллокаций памяти.

Используйте [`wheels::IntrusiveList`](https://gitlab.com/Lipovsky/wheels/-/tree/master/wheels/intrusive/list.hpp).

### Планировщик

Если ваша реализация планировщика поддерживает интрузивные задачи, то единственными динамическими аллокациями в рантайме файберов окажутся аллокации самого объекта `Fiber`.

## Синхронизация

В классе `Fiber` не должно быть дополнительной синхронизации: ни взаимного исключения, ни ожидания других потоков.

## Lock-free `Mutex`

Бонусный уровень!

Реализуйте `Mutex` без использования взаимного исключения между _потоками_ планировщика (а значит без `FutexLike`), с помощью
лок-фри стека.

## Lock-free `WaitGroup`

Бонусный уровень!

Реализуйте лок-фри `WaitGroup` с максимально быстрыми `Add` и `Done`, без взаимного исключения на быстром пути.


### Серийный мьютекс

Реализуйте серийный запуск критических секций в лок-фри мьютексе.

## Шаги

1) Реализуйте `FutexLike`. Функцию `Suspend` для начала можно подстроить под него.
2) Реализуйте через фьютекс примитивы синхронизации: `Mutex`, `CondVar` и `WaitGroup`.
3) Реализуйте универсальный `Suspend` с awaiter-ом.
4) Улучшите свою реализацию `FutexLike`: избавьтесь от аллокаций, спрячьте `Fiber`.
5) Выразите `Yield` через `Suspend`.
6) Бонусный уровень: реализуйте лок-фри мьютекс.
7) Бонусный уровень: реализуйте лок-фри `WaitGroup`.
8) Бонусный уровень: реализуйте серийный запуск критических секций в мьютексе.

## Misc

Не меняйте API пула потоков, файберы для планирования полагаются только на метод `Submit`.

Не делайте `FiberHandle` узлом интрузивного списка. Интрузивный список – это деталь реализации фьютекса, и будет странно, если она проникнет в реализацию `Yield`.

`FiberHandle` – _TriviallyCopyable_, передавайте его по значению.