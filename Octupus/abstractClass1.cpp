#include <iostream>
#include <corecrt_math_defines.h>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <iomanip>
#include <typeinfo>

struct TableRow
{
    const void* address = nullptr;
    std::string staticType;
    std::string dynamicType;
};

class Shape
{
public:
    virtual ~Shape() = default;

    virtual double area() const = 0;
    virtual void draw() const = 0;
};

class ShapeChild : public Shape
{
public:
    explicit ShapeChild(double area)
        : _area(area)
    {
    }
    // ~ShapeChild() override = default; деструктор генерируется автоматически
    double area() const override
    {
        return _area;
    }
    void draw() const override
    {
        std::cout << "area: " << area() << '\n';
    }
private:
    double _area = 0.;
};

class Rectangle : public ShapeChild
{
public:
    Rectangle(double width, double height)
        : ShapeChild(width* height)
    {
    }
    static std::unique_ptr<Rectangle> create(double width, double height)
    {
        return std::make_unique<Rectangle>(width, height);
    }
    void draw() const override
    {
        std::cout << "Rectangle area: " << area() << '\n';
    }
};

class Circle : public ShapeChild
{
public:
    Circle(double radius)
        : ShapeChild(radius * radius * M_PI)
    {
    }
    static std::unique_ptr<Circle> create(double radius)
    {
        return std::make_unique<Circle>(radius);
    }
    void draw() const override
    {
        std::cout << "Circle area: " << area() << '\n';
    }
};

void printShape(const Shape& shape)
{
    shape.draw();
    std::cout << "area from printShape: " << shape.area() << '\n';
}

template <typename T>
std::string getStaticTypeName()
{
    return typeid(T).name();
}

template <typename T>
std::string getDynamicTypeName(const T& obj)
{
    return typeid(obj).name();
}

template <typename T>
void emplaceToTable(const T& obj, const std::string& name, std::map<std::string, TableRow>& tableOfAddresses)
{
    tableOfAddresses.emplace(
        name,
        TableRow{
            static_cast<const void*>(&obj),
            getStaticTypeName<T>(),
            getDynamicTypeName(obj)
        });
}

template <typename T>
void emplaceToTable(T* ptr, const std::string& name, std::map<std::string, TableRow>& tableOfAddresses)
{
    tableOfAddresses.emplace(
        name,
        TableRow{
            static_cast<const void*>(ptr),
            getStaticTypeName<T*>(),
            ptr ? typeid(*ptr).name() : "nullptr"
        });
}

template <typename T>
void emplaceToTable(const std::unique_ptr<T>& ptr, const std::string& name, std::map<std::string, TableRow>& tableOfAddresses)
{
    tableOfAddresses.emplace(
        name,
        TableRow{
            static_cast<const void*>(ptr.get()),
            "std::unique_ptr<" + getStaticTypeName<T>() + ">",
            ptr ? typeid(*ptr).name() : "nullptr   "
        });
}


void printTable(const std::map<std::string, TableRow>& tableOfAddresses)
{
    constexpr int nameWidth = 34;
    constexpr int addrWidth = 28;
    constexpr int staticTypeWidth = 34;
    constexpr int dynamicTypeWidth = 20;


    std::cout << '\n';
    std::cout << std::left
        << std::setw(nameWidth) << "name"
        << std::setw(addrWidth) << "address"
        << std::setw(staticTypeWidth) << "static type"
        << std::setw(dynamicTypeWidth) << "dynamic type"
        << '\n';

    std::cout << std::string(nameWidth + addrWidth + staticTypeWidth + dynamicTypeWidth, '-') << '\n';

    for (const auto& [name, row] : tableOfAddresses)
    {
        std::ostringstream addrStream;
        addrStream << row.address;

        std::cout << std::left
            << std::setw(nameWidth) << name
            << std::setw(addrWidth) << addrStream.str()
            << std::setw(staticTypeWidth) << row.staticType
            << std::setw(dynamicTypeWidth) << row.dynamicType
            << '\n';
    }
}

int main()
{
    std::map <std::string, TableRow> tableOfAdreses;
    ShapeChild shch{ 5.456 };  // создаём объект производного класса ShapeChild
    shch.draw();             // прямой вызов метода у объекта ShapeChild
    emplaceToTable(shch, "ShapeChild shch", tableOfAdreses);
    // std::make_unique<ShapeChild>(15.137) создаёт unique_ptr<ShapeChild>
    // далее происходит неявное преобразование unique_ptr<ShapeChild> -> unique_ptr<Shape>
    // это upcast: производный тип приводится к базовому
    std::unique_ptr<Shape> shape = std::make_unique<ShapeChild>(15.137);
    emplaceToTable(shape, "std::unique_ptr<Shape> shape", tableOfAdreses);
    // shape имеет тип unique_ptr<Shape>, но внутри хранит объект ShapeChild
    // draw() виртуальный, поэтому будет вызван именно ShapeChild::draw()
    shape->draw();

    // &shch имеет тип ShapeChild*
    // здесь происходит неявное преобразование ShapeChild* -> Shape*
    // это тоже upcast: указатель на производный класс приводится к указателю на базовый
    Shape* base = &shch;
    emplaceToTable(base, "Shape* base", tableOfAdreses);
    Circle cirxle1{ 5. };
    emplaceToTable(cirxle1, "Circle cirxle1", tableOfAdreses);
    Shape* baseCircle = &cirxle1;
    emplaceToTable(baseCircle, "Shape* baseCircle", tableOfAdreses);
    std::uintptr_t addr1 = reinterpret_cast<std::uintptr_t>(&shch);
    std::cout << "addr1: " << std::hex << std::showbase << addr1 << '\n';
    std::uintptr_t addr2 = reinterpret_cast<std::uintptr_t>(base);
    std::cout << "addr2: " << std::hex << std::showbase << addr2 << '\n';
    std::ptrdiff_t offset = static_cast<std::ptrdiff_t>(addr2 - addr1);
    std::cout << "upcast offset: " << std::hex << std::showbase << offset << '\n';
    // dynamic_cast пытается преобразовать Shape* -> ShapeChild*
    // это downcast: из базового типа обратно в производный
    // если объект реально ShapeChild, получим валидный указатель
    // если объект другого типа-наследника, получим nullptr
    Circle* derived = dynamic_cast<Circle*>(baseCircle);
    emplaceToTable(derived, "Circle* derived", tableOfAdreses);
    Circle* derivedC = dynamic_cast<Circle*>(base); // NULLPTR!
    emplaceToTable(derivedC, "Circle* derivedС", tableOfAdreses);
    /*DANGER!*/Circle* derivedCS = static_cast<Circle*>(base); // ОПАСНО! // UB-risk: проверки реального типа нет
    emplaceToTable(derivedCS, "Circle* derivedСS", tableOfAdreses);
    // проверяем, удалось ли безопасное downcast-преобразование
    if (derived)
    {
        // derived снова имеет тип ShapeChild*
        // вызывается метод производного класса
        derived->draw();

        std::uintptr_t addr1 = reinterpret_cast<std::uintptr_t>(baseCircle);
        std::cout << "addr1: " << std::hex << std::showbase << addr1 << '\n';
        std::uintptr_t addr2 = reinterpret_cast<std::uintptr_t>(derived);
        std::cout << "addr2: " << std::hex << std::showbase << addr2 << '\n';
        std::ptrdiff_t offset = static_cast<std::ptrdiff_t>(addr2 - addr1);
        std::cout << "downcast offset: " << std::hex << std::showbase << offset << '\n';
    }

    Rectangle rect{ 10, 20 };
    emplaceToTable(rect, "Rectangle rect", tableOfAdreses);
    rect.draw();

    Circle circle{ 2. };
    emplaceToTable(circle, "Circle circle", tableOfAdreses);
    Shape* baseCircle2 = &circle;
    emplaceToTable(baseCircle2, "Shape* baseCircle2", tableOfAdreses);
    baseCircle->draw();

    printShape(rect);
    printShape(circle);

    auto circle2 = Circle::create(22.);
    emplaceToTable(circle2, "auto circle2", tableOfAdreses);
    auto rect2 = Rectangle::create(2., 3.);
    emplaceToTable(rect2, "auto rect2", tableOfAdreses);
    printShape(*rect2);
    printShape(*circle2);

    printTable(tableOfAdreses);
    return 0;
}