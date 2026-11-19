#pragma once

// Requires an active 1200x760 EasyX window with batch drawing.
// The caller owns the window lifecycle and must set the bank data directory
// as the current working directory before calling.
void runBankGui();
