-- ========================================================
-- Qt 考勤管理系统数据库初始化脚本
-- 数据库：attendance_db
-- 字符集：utf8mb4
-- ========================================================

-- 创建数据库
CREATE DATABASE IF NOT EXISTS attendance_system
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE attendance_system;

-- ========================================================
-- 1. 班级表 classes（可选）
-- ========================================================
DROP TABLE IF EXISTS class_students;
DROP TABLE IF EXISTS classes;

CREATE TABLE classes (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '班级自增ID',
    class_name VARCHAR(50) NOT NULL UNIQUE COMMENT '班级名称，如：计算机2201',
    department VARCHAR(50) DEFAULT NULL COMMENT '院系',
    grade VARCHAR(20) DEFAULT NULL COMMENT '年级',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_class_name (class_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='班级信息表';

-- ========================================================
-- 2. 学生信息表 students
-- ========================================================
DROP TABLE IF EXISTS students;

CREATE TABLE students (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '自增ID',
    student_no VARCHAR(32) NOT NULL UNIQUE COMMENT '学号，唯一标识',
    name VARCHAR(50) NOT NULL COMMENT '学生姓名',
    gender TINYINT DEFAULT 0 COMMENT '性别：0-未知，1-男，2-女',
    ethnicity VARCHAR(30) DEFAULT NULL COMMENT '民族',
    id_card VARCHAR(18) DEFAULT NULL UNIQUE COMMENT '身份证件号（建议加密存储）',
    face_feature LONGBLOB DEFAULT NULL COMMENT '人脸特征值二进制数据',
    face_feature_size INT DEFAULT 0 COMMENT '人脸特征值字节长度',
    class_name VARCHAR(50) DEFAULT NULL COMMENT '所属班级名称',
    college VARCHAR(50) DEFAULT NULL COMMENT '所属学院',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    INDEX idx_name (name),
    INDEX idx_class_name (class_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='学生信息表';

-- ========================================================
-- 3. 学生考勤信息表 attendance_records
-- ========================================================
DROP TABLE IF EXISTS attendance_records;

CREATE TABLE attendance_records (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '自增ID',
    student_no VARCHAR(32) NOT NULL COMMENT '学号',
    attendance_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '考勤时间',
    attendance_type TINYINT DEFAULT 0 COMMENT '考勤类型：0-正常，1-迟到，2-早退，3-缺勤',
    check_mode TINYINT DEFAULT 0 COMMENT '打卡方式：0-人脸识别，1-手动补录',
    remark VARCHAR(200) DEFAULT NULL COMMENT '备注',

    INDEX idx_student_no (student_no),
    INDEX idx_attendance_time (attendance_time),
    INDEX idx_time_student (attendance_time, student_no)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='学生考勤信息表';

-- ========================================================
-- 4. 班级-学生关系表 class_students（可选，用于多对多）
-- ========================================================
CREATE TABLE class_students (
    class_id INT NOT NULL COMMENT '班级ID',
    student_no VARCHAR(32) NOT NULL COMMENT '学号',
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '加入时间',
    PRIMARY KEY (class_id, student_no),
    CONSTRAINT fk_cs_class_id FOREIGN KEY (class_id) REFERENCES classes(id) ON DELETE CASCADE,
    CONSTRAINT fk_cs_student_no FOREIGN KEY (student_no) REFERENCES students(student_no) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='班级学生关系表';

-- ========================================================
-- 5. 课程信息表 courses
-- ========================================================
CREATE TABLE courses (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '自增ID',
    course_no VARCHAR(32) NOT NULL UNIQUE COMMENT '课程号',
    course_name VARCHAR(100) NOT NULL COMMENT '课程名称',
    credit FLOAT DEFAULT 0 COMMENT '学分',
    semester VARCHAR(20) DEFAULT NULL COMMENT '学期',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_course_no (course_no)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='课程信息表';

-- ========================================================
-- 6. 学生成绩表 grades
-- ========================================================
CREATE TABLE grades (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '自增ID',
    student_no VARCHAR(32) NOT NULL COMMENT '学号',
    course_no VARCHAR(32) NOT NULL COMMENT '课程号',
    score FLOAT DEFAULT 0 COMMENT '成绩',
    record_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '录入时间',
    UNIQUE KEY uk_student_course (student_no, course_no),
    INDEX idx_grade_student_no (student_no),
    INDEX idx_grade_course_no (course_no)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='学生成绩表';

-- ========================================================
-- 7. 示例数据（可选，开发测试时使用）
-- ========================================================

-- 示例班级
INSERT INTO classes (class_name, department, grade) VALUES
('计算机2201', '计算机学院', '2022'),
('计算机2202', '计算机学院', '2022'),
('软件工程2201', '软件学院', '2022');

-- 示例学生（身份证号为虚构测试数据）
INSERT INTO students (student_no, name, gender, ethnicity, id_card, class_name, college) VALUES
('2022010001', '张三', 1, '汉族', 'DEMO-ID-001', '计算机2201', '计算机学院'),
('2022010002', '李四', 2, '汉族', 'DEMO-ID-002', '计算机2201', '计算机学院'),
('2022010003', '王五', 1, '回族', 'DEMO-ID-003', '计算机2202', '计算机学院'),
('2022010004', '赵六', 2, '满族', 'DEMO-ID-004', '软件工程2201', '软件学院');

-- 示例课程
INSERT INTO courses (course_no, course_name, credit, semester) VALUES
('CS101', '数据结构', 4.0, '2023-2024-1'),
('CS102', '操作系统', 3.5, '2023-2024-1'),
('SE101', '软件工程', 3.0, '2023-2024-2');

-- 示例成绩
INSERT INTO grades (student_no, course_no, score) VALUES
('2022010001', 'CS101', 88.5),
('2022010001', 'CS102', 92.0),
('2022010002', 'CS101', 76.0),
('2022010003', 'CS101', 85.5),
('2022010004', 'SE101', 90.0);

-- 示例考勤记录
INSERT INTO attendance_records (student_no, attendance_time, attendance_type, check_mode) VALUES
('2022010001', '2026-07-08 08:30:00', 0, 0),
('2022010002', '2026-07-08 08:31:20', 0, 0),
('2022010003', '2026-07-08 08:35:10', 1, 0),
('2022010001', '2026-07-07 08:28:00', 0, 0),
('2022010002', '2026-07-07 08:29:30', 0, 0);

-- ========================================================
-- 8. 常用统计视图/查询示例
-- ========================================================

-- 查询某日已出勤学生列表
-- SELECT DISTINCT student_no FROM attendance_records WHERE DATE(attendance_time) = '2026-07-08';

-- 统计某日出勤/未出勤人数
-- SELECT
--     COUNT(DISTINCT ar.student_no) AS attended_count,
--     (SELECT COUNT(*) FROM students) - COUNT(DISTINCT ar.student_no) AS absent_count
-- FROM attendance_records ar
-- WHERE DATE(ar.attendance_time) = '2026-07-08';

-- 按班级统计某日考勤
-- SELECT
--     s.class_name,
--     COUNT(DISTINCT ar.student_no) AS attended,
--     COUNT(DISTINCT s.student_no) - COUNT(DISTINCT ar.student_no) AS absent
-- FROM students s
-- LEFT JOIN attendance_records ar
--     ON s.student_no = ar.student_no AND DATE(ar.attendance_time) = '2026-07-08'
-- GROUP BY s.class_name;
