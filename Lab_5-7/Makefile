# Makefile

# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall
LDFLAGS = -lzmq

# Названия исполняемых файлов
COMPUTING_EXE = computing_node
MANAGER_EXE = manager

# Исходные файлы
COMPUTING_SRC = computing_node.cpp
MANAGER_SRC = manager_node.cpp main.cpp

# Заголовочные файлы
COMPUTING_HDR = computing_node.h
MANAGER_HDR = manager_node.h tree_node.h

# Цель по умолчанию
all: $(COMPUTING_EXE) $(MANAGER_EXE)

# Правило для сборки вычислительного узла
$(COMPUTING_EXE): $(COMPUTING_SRC) $(COMPUTING_HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(COMPUTING_SRC) $(LDFLAGS)

# Правило для сборки управляющего узла
$(MANAGER_EXE): $(MANAGER_SRC) $(MANAGER_HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(MANAGER_SRC) $(LDFLAGS)

# Правило для очистки сборки
clean:
	rm -f $(COMPUTING_EXE) $(MANAGER_EXE)
