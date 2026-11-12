# 数据库设计文档

---

## 1. 数据库选型

- **数据库**：MySQL 8.0（或 MariaDB 10.5+）
- **字符集**：utf8mb4
- **排序规则**：utf8mb4_unicode_ci
- **时区**：建议服务器使用中国时区（Asia/Shanghai），或统一使用 UTC 存储、本地时区显示

---

## 2. 数据库 E-R 关系

```text
┌─────────────┐       ┌─────────────────────┐
│   classes   │       │      students       │
│  (班级表)    │1    * │    (学生信息表)      │
└──────┬──────┘       └──────────┬──────────┘
       │                         │
       │                         │1
       │                         │
       │                    ┌────┴────┐
       │                    │    *    │
       │              ┌─────┴─────────┐
       │              │ attendance_records │
       │              │  (考勤记录表)   │
       │              └─────────────────┘
       │
       └───────────────┐
                       │
              ┌────────┴────────┐
              │  class_students   │
              │ (班级-学生关系表) │
              └─────────────────┘
```

> 说明：为简化实现，可直接在 `students` 表中冗余 `class_name` 字段；如班级信息较复杂，建议拆分为 `classes` 表与 `class_students` 关系表。

---

## 3. 数据表结构

### 3.1 班级表 classes（可选）

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| id | INT | PRIMARY KEY AUTO_INCREMENT | 班级自增 ID |
| class_name | VARCHAR(50) | NOT NULL UNIQUE | 班级名称，如“计算机 2201” |
| department | VARCHAR(50) | NULL | 院系 |
| grade | VARCHAR(20) | NULL | 年级 |
| created_at | DATETIME | DEFAULT CURRENT_TIMESTAMP | 创建时间 |

### 3.2 学生信息表 students

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| id | INT | PRIMARY KEY AUTO_INCREMENT | 自增 ID |
| student_no | VARCHAR(32) | NOT NULL UNIQUE | 学号，唯一标识 |
| name | VARCHAR(50) | NOT NULL | 学生姓名 |
| gender | TINYINT | DEFAULT 0 | 性别：0-未知，1-男，2-女 |
| ethnicity | VARCHAR(30) | NULL | 民族，如“汉族” |
| id_card | VARCHAR(18) | NULL UNIQUE | 身份证件号，建议加密存储 |
| face_feature | LONGBLOB | NULL | 人脸特征值（二进制） |
| face_feature_size | INT | DEFAULT 0 | 特征值字节长度 |
| class_name | VARCHAR(50) | NULL | 所属班级名称 |
| created_at | DATETIME | DEFAULT CURRENT_TIMESTAMP | 创建时间 |
| updated_at | DATETIME | DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | 更新时间 |

**索引建议**：

```sql
CREATE UNIQUE INDEX uk_student_no ON students(student_no);
CREATE INDEX idx_class_name ON students(class_name);
CREATE INDEX idx_name ON students(name);
```

### 3.3 学生考勤信息表 attendance_records

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| id | BIGINT | PRIMARY KEY AUTO_INCREMENT | 自增 ID |
| student_no | VARCHAR(32) | NOT NULL | 学号，关联 students.student_no |
| attendance_time | DATETIME | DEFAULT CURRENT_TIMESTAMP | 考勤时间 |
| attendance_type | TINYINT | DEFAULT 0 | 考勤类型：0-正常，1-迟到，2-早退，3-缺勤 |
| check_mode | TINYINT | DEFAULT 0 | 打卡方式：0-人脸识别，1-手动补录 |
| remark | VARCHAR(200) | NULL | 备注 |

**索引建议**：

```sql
CREATE INDEX idx_student_no ON attendance_records(student_no);
CREATE INDEX idx_attendance_time ON attendance_records(attendance_time);
CREATE INDEX idx_time_student ON attendance_records(attendance_time, student_no);
```

### 3.4 班级-学生关系表 class_students（可选，用于多对多）

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| class_id | INT | NOT NULL | 班级 ID |
| student_no | VARCHAR(32) | NOT NULL | 学号 |
| PRIMARY KEY | (class_id, student_no) | | 联合主键 |

---

## 4. 核心查询示例

### 4.1 查询某日已出勤学生

```sql
SELECT DISTINCT student_no
FROM attendance_records
WHERE DATE(attendance_time) = '2026-07-08';
```

### 4.2 统计某日出勤人数与未出勤人数

```sql
SELECT
    COUNT(DISTINCT ar.student_no) AS attended_count,
    (SELECT COUNT(*) FROM students) - COUNT(DISTINCT ar.student_no) AS absent_count
FROM attendance_records ar
WHERE DATE(ar.attendance_time) = '2026-07-08';
```

### 4.3 按班级统计某日考勤

```sql
SELECT
    s.class_name,
    COUNT(DISTINCT ar.student_no) AS attended,
    COUNT(DISTINCT s.student_no) - COUNT(DISTINCT ar.student_no) AS absent
FROM students s
LEFT JOIN attendance_records ar
    ON s.student_no = ar.student_no AND DATE(ar.attendance_time) = '2026-07-08'
GROUP BY s.class_name;
```

### 4.4 查询学生某月考勤记录

```sql
SELECT *
FROM attendance_records
WHERE student_no = '2022010001'
  AND attendance_time >= '2026-07-01 00:00:00'
  AND attendance_time < '2026-08-01 00:00:00'
ORDER BY attendance_time DESC;
```

---

## 5. 数据安全建议

1. **身份证脱敏/加密**：如非必要不存储原始身份证号；必须存储时采用 AES 加密。
2. **人脸特征安全**：人脸特征不可逆还原为人脸图像，但仍应限制数据库访问权限。
3. **数据库账号权限**：应用程序使用独立 MySQL 账号，仅授予 INSERT/SELECT/UPDATE/DELETE 权限，不授予 DROP。
4. **连接加密**：生产环境建议启用 SSL/TLS 连接数据库。
