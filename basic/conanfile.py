from conan import ConanFile
from conan.tools.cmake import cmake_layout


class CppRtcPracticeWebConan(ConanFile):
    name = "cpp-rtc-practice-web"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("crowcpp-crow/1.2.1")

    def configure(self):
        self.options["crowcpp-crow"].with_ssl = False

    def layout(self):
        cmake_layout(self)
