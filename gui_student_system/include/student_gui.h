#pragma once

#include "student.h"

// Requires an active 1200x760 EasyX window; caller owns its lifecycle.
void runStudentGui(const Student& student);
void runStudentGui(const string& userid);
