cmake_minimum_required(VERSION 3.4)
project(CourseWork)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(pmols)
add_subdirectory(mols_packer)
<<<<<<< HEAD
add_subdirectory(tests)
=======
add_subdirectory(mol_visualizer)
add_subdirectory(tests)
>>>>>>> 038e334a388126d42b0fb0c2c05aa260f5dd3043
