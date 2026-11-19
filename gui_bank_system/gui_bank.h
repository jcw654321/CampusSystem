#pragma once

#include <filesystem>

namespace gui_bank {

// Blocks until the GUI closes. dataDirectory must contain (or be writable for)
// accounts.txt and transactions.txt. Call from the UI thread. The process
// working directory is temporarily changed and restored before this returns.
bool run(const std::filesystem::path& dataDirectory);

} // namespace gui_bank
