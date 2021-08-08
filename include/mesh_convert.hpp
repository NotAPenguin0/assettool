#pragma once

#include <plib/stream.hpp>
#include <iostream>

bool convert_mesh(plib::binary_input_stream& in, plib::binary_output_stream& out, std::ostream& log);