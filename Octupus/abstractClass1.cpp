#include <iostream>
#include <ios>
#include <memory>
#include <map>
#include <sstream>
#include <cstdarg>
#include <string>
#include <iomanip>
#include <typeinfo>

#include <cstdio>

#include <Windows.h>
#include <gl/GL.h>

#pragma comment(lib, "opengl32.lib")

constexpr double Pi = 3.1415926535897932384626433832795;

void RunShapeDemo();
namespace
{
    HGLRC g_hGLRC = nullptr;

    void CreateDebugConsole()
    {
        if (!AllocConsole())
            return;

        FILE* fp = nullptr;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);

        std::ios::sync_with_stdio(true);

        std::cout.clear();
        std::cerr.clear();
        std::cin.clear();

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        std::wcout.clear();
        std::wcerr.clear();
        std::wcin.clear();
    }

    bool SetupPixelFormat(HDC hdc)
    {
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;

        const int pixelFormat = ChoosePixelFormat(hdc, &pfd);
        if (pixelFormat == 0)
            return false;

        if (!SetPixelFormat(hdc, pixelFormat, &pfd))
            return false;

        return true;
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_SIZE:
        {
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);

            if (height > 0)
            {
                const double aspect = static_cast<double>(width) / static_cast<double>(height);

                glViewport(0, 0, width, height);

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                const double zNear = 1.0;
                const double zFar = 100.0;
                const double fovY = 45.0;
                const double top = zNear * tan(fovY * Pi / 360.0);
                const double bottom = -top;
                const double right = top * aspect;
                const double left = -right;

                glFrustum(left, right, bottom, top, zNear, zFar);

                glMatrixMode(GL_MODELVIEW);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd, &ps);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            SwapBuffers(hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            if (g_hGLRC != nullptr)
            {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(g_hGLRC);
                g_hGLRC = nullptr;
            }

            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
void DrawCube()
{
    glBegin(GL_QUADS);

    // Front
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Back
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    // Left
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    // Right
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);

    // Top
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    // Bottom
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    glEnd();
}
void RenderFrame(HDC hdc)
{
    static float angle = 0.0f;
    angle += 0.3f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -6.0f);
    glRotatef(angle, 30.0f, 45.0f, 0.0f);

    DrawCube();

    SwapBuffers(hdc);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    CreateDebugConsole();
    std::cout << "Console attached.\n";
    RunShapeDemo();
    const wchar_t kClassName[] = L"OpenGLBlackWindowClass";

    WNDCLASSW wc{};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassW(&wc))
        return -1;

    HWND hwnd = CreateWindowExW(
        0,
        kClassName,
        L"OpenGL Black Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr)
        return -2;

    HDC hdc = GetDC(hwnd);
    if (hdc == nullptr)
        return -3;

    if (!SetupPixelFormat(hdc))
    {
        ReleaseDC(hwnd, hdc);
        return -4;
    }

    g_hGLRC = wglCreateContext(hdc);
    if (g_hGLRC == nullptr)
    {
        ReleaseDC(hwnd, hdc);
        return -5;
    }

    if (!wglMakeCurrent(hdc, g_hGLRC))
    {
        wglDeleteContext(g_hGLRC);
        g_hGLRC = nullptr;
        ReleaseDC(hwnd, hdc);
        return -6;
    }
    glEnable(GL_DEPTH_TEST);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (true)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
        {
            RenderFrame(hdc);
        }
    }

    ReleaseDC(hwnd, hdc);
    FreeConsole();
    return static_cast<int>(msg.wParam);
}
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

class RectangleMy : public ShapeChild
{
public:
    RectangleMy(double width, double height)
        : ShapeChild(width* height)
    {
    }
    static std::unique_ptr<RectangleMy> create(double width, double height)
    {
        return std::make_unique<RectangleMy>(width, height);
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
        : ShapeChild(radius * radius * Pi)
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

static void printShape(const Shape& shape)
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


static void printTable(const std::map<std::string, TableRow>& tableOfAddresses)
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

void RunShapeDemo()
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

    RectangleMy rect{ 10, 20 };
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
    auto rect2 = RectangleMy::create(2., 3.);
    emplaceToTable(rect2, "auto rect2", tableOfAdreses);
    printShape(*rect2);
    printShape(*circle2);

    printTable(tableOfAdreses);
}