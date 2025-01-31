# Nom de l'exécutable
EXECUTABLE = myapp

# Compilateur et options
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Fichiers sources
SOURCES = \
    Correc_Prof/game.cpp \
    Correc_Prof/gridmanagement.cpp \
    main.cpp

# Générez une liste d'objets à partir des sources
OBJECTS = $(SOURCES:.cpp=.o)

# Règle par défaut : construire l'exécutable
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)

# Règle générique pour la compilation des .cpp en .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage des fichiers intermédiaires et de l'exécutable
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Optionnel : installation ou autres commandes
install: $(EXECUTABLE)
	@echo "Installation pas définie."

.PHONY: all clean install
