/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>

int main(int argc, char* argv[]) {
  size_t M = 1024;

  try {
    if (argc > 1) {
      M = std::stoi(argv[1]);
    }
  } catch (std::invalid_argument const &ex) {
    std::cout << "Invalid number: " << argv[1] << std::endl;
  } catch (std::out_of_range const &ex) {
    std::cout << "Number out of range: " << argv[1] << std::endl;
  }

  adiar::adiar_init(M * 1024 * 1024);

  {
    // ===== Your code starts here =====



    // =====  Your code ends here  =====
  }

  adiar::adiar_printstat();

  adiar::adiar_deinit();
  exit(0);
}

