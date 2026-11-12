cmake_minimum_required(VERSION 3.16)
project(AttendanceSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Charts Multimedia)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/StudentManageWidget.cpp
    src/ui/StudentManageWidget.h
    src/ui/StudentDialog.cpp
    src/ui/StudentDialog.h
    src/ui/GradeManageWidget.cpp
    src/ui/GradeManageWidget.h
    src/ui/GradeDialog.cpp
    src/ui/GradeDialog.h
    src/ui/CourseDialog.cpp
    src/ui/CourseDialog.h
    src/ui/FaceAttendanceWidget.cpp
    src/ui/FaceAttendanceWidget.h
    src/ui/FaceEnrollmentDialog.cpp
    src/ui/FaceEnrollmentDialog.h
    src/ui/StatisticsWidget.cpp
    src/ui/StatisticsWidget.h
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/database/StudentDao.cpp
    src/database/StudentDao.h
    src/database/CourseDao.cpp
    src/database/CourseDao.h
    src/database/GradeDao.cpp
    src/database/GradeDao.h
    src/database/AttendanceDao.cpp
    src/database/AttendanceDao.h
    src/models/Student.cpp
    src/models/Student.h
    src/models/Course.cpp
    src/models/Course.h
    src/models/Grade.cpp
    src/models/Grade.h
    src/models/AttendanceRecord.cpp
    src/models/AttendanceRecord.h
    src/face/FaceEngine.cpp
    src/face/FaceEngine.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::Multimedia
)

# ============================================================================
# Optional: SeetaFace6 integration
# ============================================================================
# Set SEETA_FACE_DIR to your SeetaFace6 SDK root directory, e.g.:
#   cmake -S . -B build -DSEETA_FACE_DIR="C:/SeetaFace6/sf6.0_windows"
# The SDK should contain:
#   include/  — header files (seeta/FaceDetector.h, etc.)
#   lib/      — .lib and .dll files
# Model files (.csta) should be placed alongside the executable or in sf6.0_windows/.
# ============================================================================
set(SEETA_FACE_DIR "" CACHE PATH "SeetaFace6 SDK root directory")

if (SEETA_FACE_DIR)
    message(STATUS "Searching for SeetaFace6 in ${SEETA_FACE_DIR}")
    # MinGW places libs in lib/x64, MSVC in lib/x64 or lib
    find_library(SEETA_FD_LIB NAMES SeetaFaceDetector600 SeetaFaceDetector
        PATHS "${SEETA_FACE_DIR}/lib/x64" "${SEETA_FACE_DIR}/lib" NO_DEFAULT_PATH)
    find_library(SEETA_FL_LIB NAMES SeetaFaceLandmarker600 SeetaFaceLandmarker
        PATHS "${SEETA_FACE_DIR}/lib/x64" "${SEETA_FACE_DIR}/lib" NO_DEFAULT_PATH)
    find_library(SEETA_FR_LIB NAMES SeetaFaceRecognizer610 SeetaFaceRecognizer
        PATHS "${SEETA_FACE_DIR}/lib/x64" "${SEETA_FACE_DIR}/lib" NO_DEFAULT_PATH)

    if (SEETA_FD_LIB AND SEETA_FL_LIB AND SEETA_FR_LIB)
        message(STATUS "SeetaFace6 libraries found:")
        message(STATUS "  FaceDetector:   ${SEETA_FD_LIB}")
        message(STATUS "  FaceLandmarker: ${SEETA_FL_LIB}")
        message(STATUS "  FaceRecognizer: ${SEETA_FR_LIB}")

        target_include_directories(${PROJECT_NAME} PRIVATE "${SEETA_FACE_DIR}/include")
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SEETA_FD_LIB} ${SEETA_FL_LIB} ${SEETA_FR_LIB})
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SEETA_FACE6)
        message(STATUS "SeetaFace6 integration enabled. FaceEngine will use real SDK APIs.")
    else()
        message(WARNING "SeetaFace6 not found in ${SEETA_FACE_DIR}. "
                        "Searched: SeetaFaceDetector(600), SeetaFaceLandmarker(600), SeetaFaceRecognizer(610). "
                        "Building without SeetaFace6 — FaceEngine will be a placeholder.")
    endif()
else()
    message(STATUS "SEETA_FACE_DIR not set. Building without SeetaFace6. "
                    "Set -DSEETA_FACE_DIR=... to enable real face recognition.")
endif()

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
