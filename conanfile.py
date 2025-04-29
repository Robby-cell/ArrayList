from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class ArrayListRecipe(ConanFile):
    name = "arraylist"
    version = "0.1"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Robert Williamson"
    url = "https://github.com/Robby-cell/ArrayList"
    description = "A C++ ArrayList implementation"
    topics = ("ArrayList", "Vector", "std::vector", "C++")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "tests/*"

    requires = ["catch2/3.8.0"]

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    

    