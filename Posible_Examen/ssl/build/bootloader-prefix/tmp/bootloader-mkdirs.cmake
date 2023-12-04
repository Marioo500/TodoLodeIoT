# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/Programs/ESP/esp-idf/components/bootloader/subproject"
  "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader"
  "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix"
  "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix/tmp"
  "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix/src"
  "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/luis0/Documents/Universidad/IoT/Posible_Examen/ssl/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
