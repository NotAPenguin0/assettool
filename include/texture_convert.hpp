#pragma once

#include <mipgen/mipgen.hpp>
#include <plib/stream.hpp>

#include <iostream>

bool convert_texture(mipgen::Context& ctx, plib::binary_input_stream& in, plib::binary_output_stream& out, std::ostream& log);