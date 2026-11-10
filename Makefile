# Компилятор
CXX = g++

# Флаги компиляции (стандарт C++17, предупреждения, оптимизация)
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Библиотеки для линковки (Boost и Crypto++)
LDLIBS = -lboost_program_options -lcryptopp

# Имя итогового исполняемого файла
TARGET = client

# Автоматический поиск всех .cpp файлов в текущей директории
SRCS = $(wildcard *.cpp)

# Преобразование имен .cpp в имена .o
OBJS = $(SRCS:.cpp=.o)

# Цель по умолчанию
all: $(TARGET)

# Линковка исполняемого файла
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# Компиляция каждого .cpp в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка сгенерированных файлов
clean:
	rm -f $(OBJS) $(TARGET)

# Указываем, что clean и all - это не файлы
.PHONY: all clean
