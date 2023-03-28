# CMakeList.txt : CMake project for NEATImplementation, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NEATImplementation "main.cpp" "NEAT/Genome.cpp" "NEAT/NeuralNetwork.cpp" "NEAT/GenomeMutation.cpp"  "NEAT/GenomeGenerator.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NEATImplementation PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
