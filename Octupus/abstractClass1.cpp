#include <iostream>
#include <corecrt_math_defines.h>
#include <memory>

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

int main()
{
    ShapeChild shch{ 5.456 };  // создаём объект производного класса ShapeChild
    shch.draw();             // прямой вызов метода у объекта ShapeChild

    // std::make_unique<ShapeChild>(15.137) создаёт unique_ptr<ShapeChild>
    // далее происходит неявное преобразование unique_ptr<ShapeChild> -> unique_ptr<Shape>
    // это upcast: производный тип приводится к базовому
    std::unique_ptr<Shape> shape = std::make_unique<ShapeChild>(15.137);

    // shape имеет тип unique_ptr<Shape>, но внутри хранит объект ShapeChild
    // draw() виртуальный, поэтому будет вызван именно ShapeChild::draw()
    shape->draw();

    // &shch имеет тип ShapeChild*
    // здесь происходит неявное преобразование ShapeChild* -> Shape*
    // это тоже upcast: указатель на производный класс приводится к указателю на базовый
    Shape* base = &shch;

    // dynamic_cast пытается преобразовать Shape* -> ShapeChild*
    // это downcast: из базового типа обратно в производный
    // если объект реально ShapeChild, получим валидный указатель
    // если объект другого типа-наследника, получим nullptr
    ShapeChild* derived = dynamic_cast<ShapeChild*>(base);

    // проверяем, удалось ли безопасное downcast-преобразование
    if (derived)
    {
        // derived снова имеет тип ShapeChild*
        // вызывается метод производного класса
        derived->draw();
    }

    Rectangle rect{ 10, 20 };
    rect.draw();

    Circle circle{ 2. };
    Shape* baseCircle = &circle;
    baseCircle->draw();

    printShape(rect);
    printShape(circle);

    auto circle2 = Circle::create(22.);
    auto rect2 = Rectangle::create(2., 3.);
    printShape(*rect2);
    printShape(*circle2);
    return 0;
}