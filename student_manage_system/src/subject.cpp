#include "subject.h"
using namespace std;

string subject::getName() const {
    return name;
}

string subject::getSubjectId() const {
    return subjectId;
}

string subject::getTeacherName() const {
    return teacherName;
}

float subject::getScore() const {
    return score;
}

void subject::setName(const string& name) {
    this->name = name;
}

void subject::setSubjectId(const string& subjectId) {
    this->subjectId = subjectId;
}

void subject::setTeacherName(const string& teacherName) {
    this->teacherName = teacherName;
}

void subject::setScore(float score) {
    this->score = score;
}