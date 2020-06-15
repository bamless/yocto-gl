//
// LICENSE:
//
// Copyright (c) 2016 -- 2020 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <yocto/yocto_commonio.h>
#include <yocto/yocto_image.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_sceneio.h>
#include <yocto/yocto_trace.h>
#include <yocto_denoise/yocto_denoise.h>
using namespace yocto;
using namespace yocto::denoise;

using namespace std::string_literals;

#include "ext/filesystem.hpp"
namespace sfs = ghc::filesystem;

#include <iostream>

int main(int argc, const char* argv[]) {
  auto outname     = "out.png"s;
  auto filename    = "img.hdr"s;
  auto albedo_name = ""s;
  auto normal_name = ""s;

  // parse cli arguments and assure their correctness
  auto cli = make_cli(
      "yimagedenoise", "Denoise images using Intel Open Image Denoise");
  add_option(cli, "--alb,-a", albedo_name, "albedo feature image filename");
  add_option(cli, "--nrm,-n", normal_name, "normal feature image filename");
  add_option(cli, "--output,-o", outname, "output image filename");
  add_option(cli, "filename", filename, "input image filename", true);
  parse_cli(cli, argc, argv);

  if (normal_name != "" && albedo_name == "")
    print_fatal("cannot use normal feature image without an albedo one");
  if (normal_name != "" && !is_hdr_filename(normal_name))
    print_fatal("normal feature image must be provided in pfm or exr format");

  // error string for yocto library functions
  auto error = ""s;

  // load all the provided images
  auto         hdr = is_hdr_filename(filename);
  image<vec3f> color, albedo, normal;

  if (!load_image(filename, color, error)) {
    print_fatal(error);
  }
  if (albedo_name != "" && !load_image(albedo_name, albedo, error)) {
    print_fatal(error);
  }
  if (normal_name != "" && !load_image(normal_name, normal, error)) {
    print_fatal(error);
  }

  // call the denoiser passing in the noisy image and the provided feature
  // images
  image<vec3f> out;
  if (albedo_name == "") {
    if (!oidn_image_denoise(color, hdr, out, error, print_progress))
      print_fatal(error);
  } else if (normal_name != "") {
    if (!oidn_image_denoise(
            color, hdr, albedo, normal, out, error, print_progress))
      print_fatal(error);
  } else {
    if (!oidn_image_denoise(color, hdr, albedo, out, error, print_progress))
      print_fatal(error);
  }

  // finally save the result
  print_progress("save image", 0, 1);
  if (!save_image(outname, out, error)) print_fatal(error);
  print_progress("save image", 1, 1);
}